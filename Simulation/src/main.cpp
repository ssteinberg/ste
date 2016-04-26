
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
#include "ObjectGroup.hpp"
#include "TextManager.hpp"
#include "AttributedString.hpp"
#include "RGB.hpp"
#include "Sphere.hpp"
#include "gpu_task.hpp"
#include "BRDF.hpp"
#include "bme_brdf_representation.hpp"

using namespace StE::Core;
using namespace StE::Text;

class SkyDome : public StE::Graphics::gpu_dispatchable {
	using Base = StE::Graphics::gpu_dispatchable;

private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;

	std::unique_ptr<Texture2D> stars_tex;
	std::shared_ptr<GLSLProgram> program;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	std::unique_ptr<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>> meshptr;

public:
	SkyDome(const StE::StEngineControl &ctx) : program(ctx.glslprograms_pool().fetch_program_task({ "transform_sky.vert", "frag_sky.frag" })()),
											   meshptr(std::make_unique<StE::Graphics::Sphere>(10, 10, .0f)) {
		stars_tex = StE::Resource::SurfaceFactory::load_texture_2d_task("Data/textures/stars.jpg", true)();

		program->set_uniform("sky_luminance", 5.f);
		program->set_uniform("projection", ctx.projection_matrix());
		program->set_uniform("near", ctx.get_near_clip());
		program->set_uniform("far", ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near, float clip_far) {
			this->program->set_uniform("projection", proj);
			this->program->set_uniform("near", clip_near);
			this->program->set_uniform("far", clip_far);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		program->set_uniform("view_model", m);
	}

protected:
	void set_context_state() const override final {
		GL::gl_current_context::get()->enable_depth_test();
		GL::gl_current_context::get()->color_mask(false, false, false, false);

		0_tex_unit = *stars_tex;
		meshptr->vao()->bind();
		meshptr->ebo()->bind();
		program->bind();
	}

	void dispatch() const override final {
		GL::gl_current_context::get()->draw_elements(GL_TRIANGLES, meshptr->ebo()->size(), GL_UNSIGNED_INT, nullptr);
	}
};

auto create_light_object(const std::shared_ptr<StE::Graphics::Scene> &scene, const glm::vec3 &light_pos, const std::shared_ptr<StE::Graphics::SphericalLight> &light) {
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

void create_glass(const std::shared_ptr<StE::Graphics::Scene> &scene, StE::StEngineControl *ctx, const glm::vec3 &p0, const glm::vec3 &p1) {
	std::shared_ptr<StE::Graphics::BRDF> brdf = StE::Graphics::bme_brdf_representation::BRDF_from_bme_representation_task(*ctx, "Data/bxdf/ward_metal/laminate_part_of_tile")();

	glm::vec3 U = glm::vec3{ p0.x, p0.y, p0.z } - glm::vec3{ p1.x, p1.y, p1.z };
	glm::vec3 V = glm::vec3{ p0.x, p1.y, p0.z } - glm::vec3{ p1.x, p1.y, p1.z };
	glm::vec3 N = glm::normalize(glm::cross(U, V));
	glm::vec3 T = glm::normalize(glm::cross(N, glm::vec3{0,1,0}));

	auto m = std::make_unique<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>>();
	std::vector<StE::Graphics::ObjectVertexData> vertices;
	StE::Graphics::ObjectVertexData v;

	v.n = N;
	v.t = T;
	v.p = { p0.x, p0.y, p0.z };
	v.uv = { 0, 0 };
	vertices.push_back(v);
	v.p = { p1.x, p0.y, p1.z };
	v.uv = { 1, 0 };
	vertices.push_back(v);
	v.p = { p0.x, p1.y, p0.z };
	v.uv = { 0, 1 };
	vertices.push_back(v);
	v.p = { p1.x, p1.y, p1.z };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	v.n = -N;
	v.t = -T;
	v.p = { p0.x, p0.y, p0.z };
	v.uv = { 0, 0 };
	vertices.push_back(v);
	v.p = { p1.x, p0.y, p1.z };
	v.uv = { 1, 0 };
	vertices.push_back(v);
	v.p = { p0.x, p1.y, p0.z };
	v.uv = { 0, 1 };
	vertices.push_back(v);
	v.p = { p1.x, p1.y, p1.z };
	v.uv = { 1, 1 };
	vertices.push_back(v);

	m->set_vertices(vertices);
	m->set_indices(std::vector<std::uint32_t>{0,2,1,1,2,3, 4,5,6,6,5,7});
	auto obj = std::make_shared<StE::Graphics::Object>(std::move(m));

	obj->set_model_matrix(glm::mat4());

	auto tex = StE::Resource::SurfaceFactory::load_surface_2d_task("Data/textures/glass.png", true)();
	auto nm = StE::Resource::SurfaceFactory::load_surface_2d_task("Data/textures/glass_normal.png", false)();
	auto alpha = StE::Resource::SurfaceFactory::load_surface_2d_task("Data/textures/glass_alpha.png", false)();
	auto spec = StE::Resource::SurfaceFactory::load_surface_2d_task("Data/textures/glass_specular.png", false)();

	auto mat = std::make_shared<StE::Graphics::Material>();
	mat->set_diffuse(std::make_shared<StE::Core::Texture2D>(tex, true));
	mat->set_specular(std::make_shared<StE::Core::Texture2D>(spec, true));
	mat->set_normalmap(std::make_shared<StE::Core::Texture2D>(nm, true));
	mat->set_alphamap(std::make_shared<StE::Core::Texture2D>(alpha, true));
	mat->set_brdf(brdf);

	obj->set_material_id(scene->scene_properties().materials_storage().add_material(mat));

	scene->object_group().add_object(obj);
}

int main() {
	StE::Log logger("Global Illumination");
//	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	// int w = 1688;
	int w = 1500;
	int h = w * 9 / 16;
	constexpr float clip_far = 3000.f;
	constexpr float clip_near = 5.f;

	GL::gl_context::context_settings settings;
	settings.vsync = false;
	settings.fs = false;
	StE::StEngineControl ctx(std::make_unique<GL::gl_context>(settings, "Shlomi Steinberg - Global Illumination", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near, clip_far);

	StE::Text::TextManager text_manager(ctx, StE::Text::Font("Data/ArchitectsDaughter.ttf"));

	using ResizeSignalConnectionType = StE::StEngineControl::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	auto scene = StE::Graphics::Scene::create(ctx);
	StE::Graphics::GIRenderer renderer(ctx, scene);

	std::unique_ptr<SkyDome> skydome = std::make_unique<SkyDome>(ctx);

	ctx.set_renderer(&renderer);

	StE::Graphics::Camera camera;
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ 26.4, 548.5, -248.71 });


	bool running = true;
	bool loaded = false;
	auto model_future = ctx.scheduler().schedule_now(StE::Resource::ModelFactory::load_model_task(ctx,
																								  R"(Data/models/crytek-sponza/sponza.obj)",
																								  &scene->object_group(),
																								  &scene->scene_properties(),
																								  2.5f));

	const glm::vec3 light0_pos{ -700.6, 138, -70 };
	const glm::vec3 light1_pos{ 200, 550, 170 };
	auto light0 = std::make_shared<StE::Graphics::SphericalLight>(6000.f, StE::Graphics::RGB({ 1.f, .57f, .16f }), light0_pos, 3.f);
	auto light1 = std::make_shared<StE::Graphics::SphericalLight>(20000.f, StE::Graphics::RGB({ 0.5f, .8f, 1.f }), light1_pos, 5.f);
	scene->scene_properties().lights_storage().add_light(light0);
	scene->scene_properties().lights_storage().add_light(light1);

	std::shared_ptr<StE::Graphics::Object> light0_obj, light1_obj;
	light0_obj = create_light_object(scene, light0_pos, light0);
	light1_obj = create_light_object(scene, light1_pos, light1);

	// create_glass(scene, &ctx, { -450, 680, -280 }, { -600, 890, -280 });
	// create_glass(scene, &ctx, { -262, 680, -280 }, { -436, 890, -280 });
	// create_glass(scene, &ctx, { -60, 680, -280 }, { -230, 890, -280 });
	// create_glass(scene, &ctx, { 90, 680, -280 }, { -60, 890, -280 });
	// create_glass(scene, &ctx, { 290, 680, -280 }, { 100, 890, -280 });
	// create_glass(scene, &ctx, { 300, 680, -280 }, { 523, 890, -280 });

	// Bind input
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
	ctx.hid_signal_keyboard().connect(keyboard_listner);
	ctx.set_pointer_hidden(true);

	auto title_text = text_manager.create_renderer();
	auto footer_text = text_manager.create_renderer();
	auto header_text = text_manager.create_renderer();

	auto title_text_task = make_gpu_task(title_text.get());

	renderer.add_gui_task(title_text_task);
	renderer.add_gui_task(make_gpu_task(footer_text.get()));
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
														 olive(std::to_wstring(workers_active)) + L" busy, " +
														 olive(std::to_wstring(workers_sleep)) + L" sleeping | " +
														 orange(std::to_wstring(pending_requests) + L" pending requests"))));
		}

		if (model_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			loaded = true;

		ctx.run_loop();
	}

	renderer.remove_gui_task(title_text_task);
	title_text = nullptr;
	title_text_task = nullptr;

	auto skydome_task = make_gpu_task(skydome.get());

	skydome_task->add_dependency(scene);

	renderer.add_gui_task(make_gpu_task(header_text.get()));
	renderer.add_task(scene);
	renderer.add_task(skydome_task);
	renderer.set_deferred_rendering_enabled(true);

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
			glm::ivec2 pp = ctx.get_pointer_position();
			glm::ivec2 center = ctx.get_backbuffer_size() / 2;
			ctx.set_pointer_position(center);
			auto diff_v = static_cast<glm::vec2>(center - pp) * time_delta * rotation_factor;
			camera.pitch_and_yaw(-diff_v.y, diff_v.x);
		}

		auto mv = camera.view_matrix();
		auto mvnt = camera.view_matrix_no_translation();

		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;

		light0->set_position(lp);

		light0_obj->set_model_matrix(glm::scale(glm::translate(glm::mat4(), lp), glm::vec3(light0->get_radius() / 2.f)));
		scene->set_model_matrix(mv);
		renderer.set_model_matrix(mv);
		skydome->set_model_matrix(mvnt);

		{
			using namespace StE::Text::Attributes;

			static unsigned tpf_count = 0;
			static float total_tpf = .0f;
			total_tpf += ctx.time_per_frame().count();
			++tpf_count;
			if (tpf_count % 5 == 0) {
				auto tpf = total_tpf / 5.f;
				total_tpf = .0f;

				auto tpf_str = std::to_wstring(tpf);
				header_text->set_text({ 30, h - 50 }, vsmall(b(stroke(dark_magenta, 1)(red(tpf_str)))) + L" ms");
			}

			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			footer_text->set_text({ 30, 20 }, vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB"))));
		}

		time += ctx.time_per_frame().count();
		if (!ctx.run_loop()) break;
	}

	return 0;
}
