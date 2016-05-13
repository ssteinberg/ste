
#include "stdafx.hpp"

#include "gl_utils.hpp"
#include "Log.hpp"
#include "Keyboard.hpp"
#include "Pointer.hpp"
#include "StEngineControl.hpp"
#include "GIRenderer.hpp"
#include "SphericalLight.hpp"
#include "DirectionalLight.hpp"
#include "ModelFactory.hpp"
#include "Camera.hpp"
#include "GLSLProgram.hpp"
#include "SurfaceFactory.hpp"
#include "Texture2D.hpp"
#include "Scene.hpp"
#include "Object.hpp"
#include "TextManager.hpp"
#include "AttributedString.hpp"
#include "RGB.hpp"
#include "Kelvin.hpp"
#include "Sphere.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"
#include "debug_gui.hpp"

using namespace StE::Core;
using namespace StE::Text;

class SkyDome : public StE::Graphics::gpu_dispatchable {
	using Base = StE::Graphics::gpu_dispatchable;

private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;

	std::shared_ptr<GLSLProgram> program;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	std::unique_ptr<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>> meshptr;

public:
	SkyDome(const StE::StEngineControl &ctx,
			StE::Graphics::Scene *scene) : program(ctx.glslprograms_pool().fetch_program_task({ "transform_sky.vert", "frag_sky.frag" })()),
										   meshptr(std::make_unique<StE::Graphics::Sphere>(10, 10, .0f)) {
		auto stars_tex = std::make_shared<StE::Core::Texture2D>(StE::Resource::SurfaceFactory::load_surface_2d_task("Data/textures/stars.jpg", true)());

		auto sky_mat = std::make_shared<StE::Graphics::Material>();
		sky_mat->set_diffuse(stars_tex);
		sky_mat->set_emission(glm::vec3(1.f));

		int material = scene->scene_properties().materials_storage().add_material(sky_mat);

		program->set_uniform("material", material);
		program->set_uniform("projection", ctx.projection_matrix());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near) {
			this->program->set_uniform("projection", proj);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

protected:
	void set_context_state() const override final {
		GL::gl_current_context::get()->enable_depth_test();
		GL::gl_current_context::get()->color_mask(false, false, false, false);

		meshptr->vao()->bind();
		meshptr->ebo()->bind();
		program->bind();
	}

	void dispatch() const override final {
		GL::gl_current_context::get()->draw_elements(GL_TRIANGLES, meshptr->ebo()->size(), GL_UNSIGNED_INT, nullptr);
	}
};

auto create_light_object(StE::Graphics::Scene *scene, const glm::vec3 &light_pos, const std::shared_ptr<StE::Graphics::SphericalLight> &light) {
	scene->scene_properties().lights_storage().add_light(light);

	std::unique_ptr<StE::Graphics::Sphere> sphere = std::make_unique<StE::Graphics::Sphere>(20, 20);
	auto light_obj = std::make_shared<StE::Graphics::Object>(std::move(sphere));

	light_obj->set_model_matrix(glm::scale(glm::translate(glm::mat4(), light_pos), glm::vec3(light->get_radius())));

	gli::texture2d light_color_tex{ gli::format::FORMAT_RGB8_UNORM_PACK8, { 1, 1 }, 1 };
	auto c = light->get_diffuse();
	*reinterpret_cast<glm::u8vec3*>(light_color_tex.data()) = glm::u8vec3(c.r * 255.5f, c.g * 255.5f, c.b * 255.5f);

	auto light_mat = std::make_shared<StE::Graphics::Material>();
	light_mat->set_diffuse(std::make_shared<StE::Core::Texture2D>(light_color_tex, false));
	light_mat->set_emission(c * light->get_luminance());

	light_obj->set_material_id(scene->scene_properties().materials_storage().add_material(light_mat));

	scene->object_group().add_object(light_obj);

	return light_obj;
}

int main() {
	StE::Log logger("Global Illumination");
//	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	int w = 1700;
	int h = w * 9 / 16;
	constexpr float clip_near = 1.f;
	constexpr float fovy = glm::pi<float>() * .225f;

	GL::gl_context::context_settings settings;
	settings.vsync = false;
	settings.fs = false;
	StE::StEngineControl ctx(std::make_unique<GL::gl_context>(settings, "Shlomi Steinberg - Global Illumination", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near);
	ctx.set_fov(fovy);

	auto font = StE::Text::Font("Data/ArchitectsDaughter.ttf");


	StE::Text::TextManager text_manager(ctx, font);

	using ResizeSignalConnectionType = StE::StEngineControl::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);


	StE::Graphics::Camera camera;
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ -5.4, 532.5, -228.71 });

	StE::Graphics::Scene scene(ctx);
	StE::Graphics::GIRenderer renderer(ctx, &camera, &scene);
	ctx.set_renderer(&renderer);

	std::unique_ptr<StE::Graphics::profiler> gpu_tasks_profiler = std::make_unique<StE::Graphics::profiler>();
	renderer.attach_profiler(gpu_tasks_profiler.get());
	std::unique_ptr<StE::Graphics::debug_gui> debug_gui_dispatchable = std::make_unique<StE::Graphics::debug_gui>(ctx, gpu_tasks_profiler.get(), font);


	std::unique_ptr<SkyDome> skydome = std::make_unique<SkyDome>(ctx, &scene);

	bool running = true;
	bool loaded = false;
	auto model_future = ctx.scheduler().schedule_now(StE::Resource::ModelFactory::load_model_task(ctx,
																								  R"(Data/models/crytek-sponza/sponza.obj)",
																								  &scene.object_group(),
																								  &scene.scene_properties(),
																								  2.5f));

	const glm::vec3 light0_pos{ -700.6, 138, -70 };
	const glm::vec3 light1_pos{ 200, 550, 170 };
	auto light0 = std::make_shared<StE::Graphics::SphericalLight>(8000.f, StE::Graphics::Kelvin(2000), light0_pos, 3.f);
	auto light1 = std::make_shared<StE::Graphics::SphericalLight>(20000.f, StE::Graphics::Kelvin(7000), light1_pos, 5.f);
	auto light0_obj = create_light_object(&scene, light0_pos, light0);
	auto light1_obj = create_light_object(&scene, light1_pos, light1);

	for (auto &v : { glm::vec3{ -622, 645, -310},
					 glm::vec3{  124, 645, -310},
					 glm::vec3{  497, 645, -310},
					 glm::vec3{ -242, 153, -310},
					 glm::vec3{  120, 153, -310},
					 glm::vec3{  124, 645,  552},
					 glm::vec3{  497, 645,  552},
					 glm::vec3{-1008, 153,  552},
					 glm::vec3{ -242, 153,  552},
					 glm::vec3{  120, 153,  552},
					 glm::vec3{  885, 153,  552} }) {
		auto wall_lamp = std::make_shared<StE::Graphics::SphericalLight>(5000.f, StE::Graphics::Kelvin(1800), v, 2.f);
		create_light_object(&scene, v, wall_lamp);
	}

	// Bind input
	bool mouse_down = false;
	auto keyboard_listner = std::make_shared<decltype(ctx)::hid_keyboard_signal_type::connection_type>(
		[&](StE::HID::keyboard::K key, int scanline, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;
		auto time_delta = ctx.time_per_frame().count();

		if (status != Status::KeyDown)
			return;

		if (key == keyboard::K::KeyESCAPE)
			running = false;
		if (key == keyboard::K::KeyPRINT_SCREEN || key == keyboard::K::KeyF12)
			ctx.capture_screenshot();
	});
	auto pointer_button_listner = std::make_shared<decltype(ctx)::hid_pointer_button_signal_type::connection_type>(
		[&](StE::HID::pointer::B b, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;

		mouse_down = b == pointer::B::Left && status == Status::KeyDown;
	});
	ctx.hid_signal_keyboard().connect(keyboard_listner);
	ctx.hid_signal_pointer_button().connect(pointer_button_listner);

	auto title_text = text_manager.create_renderer();
	auto footer_text = text_manager.create_renderer();

	auto title_text_task = make_gpu_task("title_text", title_text.get(), nullptr);

	renderer.add_gui_task(title_text_task);
	renderer.add_gui_task(make_gpu_task("footer_text", footer_text.get(), nullptr));
	renderer.set_deferred_rendering_enabled(false);


	while (!loaded && running) {
		{
			using namespace StE::Text::Attributes;
			AttributedWString str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
										   azure(large(L"Loading...\n")) +
										   orange(regular(L"By Shlomi Steinberg")));
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			auto workers_active = ctx.scheduler().get_thread_pool()->get_active_workers_count();
			auto workers_sleep = ctx.scheduler().get_thread_pool()->get_sleeping_workers_count();
			auto pending_requests = ctx.scheduler().get_thread_pool()->get_pending_requests_count();

			title_text->set_text({ w / 2, h / 2 + 100 }, str);
			footer_text->set_text({ 10, 50 },
								  line_height(32)(vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")) + L"\n" +
												  vsmall(b(L"Thread pool workers: ") +
														 olive(std::to_wstring(workers_active)) + 	L" busy, " +
														 olive(std::to_wstring(workers_sleep)) + 	L" sleeping | " +
														 orange(std::to_wstring(pending_requests) +	L" pending requests"))));
		}

		if (model_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			loaded = true;

		ctx.run_loop();
	}

	renderer.remove_gui_task(title_text_task);
	title_text = nullptr;
	title_text_task = nullptr;

	auto skydome_task = make_gpu_task("skydome", skydome.get(), nullptr);

	auto &scene_task = renderer.get_scene_task();
	skydome_task->add_dependency(scene_task);

	renderer.add_gui_task(make_gpu_task("debug_gui", debug_gui_dispatchable.get(), nullptr));
	renderer.add_task(scene_task);
	renderer.add_task(skydome_task);
	renderer.set_deferred_rendering_enabled(true);

	glm::ivec2 last_pointer_pos;
	float time = 0;
	while (running) {
		if (ctx.window_active()) {
			auto time_delta = ctx.time_per_frame().count();

			using namespace StE::HID;
			constexpr float movement_factor = 155.f;
			if (ctx.get_key_status(keyboard::K::KeyW) == Status::KeyDown)
				camera.step_forward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyS) == Status::KeyDown)
				camera.step_backward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyA) == Status::KeyDown)
				camera.step_left(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyD) == Status::KeyDown)
				camera.step_right(time_delta*movement_factor);

			constexpr float rotation_factor = .09f;
			glm::vec2(.0f);
			auto pp = ctx.get_pointer_position();
			if (mouse_down) {
				auto diff_v = static_cast<glm::vec2>(last_pointer_pos - pp) * time_delta * rotation_factor;
				camera.pitch_and_yaw(-diff_v.y, diff_v.x);
			}
			last_pointer_pos = pp;
		}

		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;
		light0->set_position(lp);
		light0_obj->set_model_matrix(glm::scale(glm::translate(glm::mat4(), lp), glm::vec3(light0->get_radius() / 2.f)));

		{
			using namespace StE::Text::Attributes;

			static unsigned tpf_count = 0;
			static float total_tpf = .0f;
			total_tpf += ctx.time_per_frame().count();
			++tpf_count;
			static float tpf = .0f;
			if (tpf_count % 5 == 0) {
				tpf = total_tpf / 5.f;
				total_tpf = .0f;
			}

			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			footer_text->set_text({ 10, 50 }, line_height(28)(vsmall(b(stroke(dark_magenta, 1)(red(std::to_wstring(tpf * 1000.f))))) + L" ms\n" +
															  vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")))));
		}

		time += ctx.time_per_frame().count();
		if (!ctx.run_loop()) break;
	}

	return 0;
}
