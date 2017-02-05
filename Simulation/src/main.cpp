
#include "stdafx.hpp"

#include "gl_utils.hpp"
#include "log.hpp"
#include "keyboard.hpp"
#include "pointer.hpp"
#include "ste_engine_control.hpp"
#include "gi_renderer.hpp"
#include "basic_renderer.hpp"
#include "sphere_light.hpp"
#include "quad_light.hpp"
#include "model_factory.hpp"
#include "camera.hpp"
#include "surface_factory.hpp"
#include "texture_2d.hpp"
#include "scene.hpp"
#include "object.hpp"
#include "text_manager.hpp"
#include "attributed_string.hpp"
#include "rgb.hpp"
#include "kelvin.hpp"
#include "sphere.hpp"
#include "gpu_task.hpp"
#include "profiler.hpp"
#include "future_collection.hpp"
#include "resource_instance.hpp"

#include <imgui/imgui.h>
#include "debug_gui.hpp"

//#define STATIC_SCENE

using namespace StE::Core;
using namespace StE::Text;

void display_loading_screen_until(StE::ste_engine_control &ctx, StE::Text::text_manager *text_manager, int *w, int *h, std::function<bool()> &&lambda) {
	StE::Graphics::basic_renderer basic_renderer(ctx);

	auto footer_text = text_manager->create_renderer();
	auto footer_text_task = make_gpu_task("footer_text", footer_text.get(), nullptr);

	auto title_text = text_manager->create_renderer();
	auto title_text_task = make_gpu_task("title_text", title_text.get(), nullptr);

	ctx.set_renderer(&basic_renderer);
	basic_renderer.add_task(title_text_task);
	basic_renderer.add_task(footer_text_task);

	while (true) {
		using namespace StE::Text::Attributes;

		attributed_wstring str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
									azure(large(L"Loading...\n")) +
									orange(regular(L"By Shlomi Steinberg")));
		auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
		auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

		auto workers_active = ctx.scheduler().get_thread_pool()->get_active_workers_count();
		auto workers_sleep = ctx.scheduler().get_thread_pool()->get_sleeping_workers_count();
		auto pending_requests = ctx.scheduler().get_thread_pool()->get_pending_requests_count();

		title_text->set_text({ *w / 2, *h / 2 + 100 }, str);
		footer_text->set_text({ 10, 50 },
							line_height(32)(vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")) + L"\n" +
											vsmall(b(L"Thread pool workers: ") +
													olive(std::to_wstring(workers_active)) + 	L" busy, " +
													olive(std::to_wstring(workers_sleep)) + 	L" sleeping | " +
													orange(std::to_wstring(pending_requests) +	L" pending requests"))));

		if (!ctx.run_loop() || !lambda())
			break;
	}
}

auto create_sphere_light_object(StE::Graphics::scene *scene, const glm::vec3 &light_pos, StE::Graphics::sphere_light *light, std::vector<std::unique_ptr<StE::Graphics::material>> &materials, std::vector<std::unique_ptr<StE::Graphics::material_layer>> &layers) {
	std::unique_ptr<StE::Graphics::sphere> sphere = std::make_unique<StE::Graphics::sphere>(20, 20);
	(*sphere) *= light->get_radius();
	auto light_obj = std::make_shared<StE::Graphics::object>(std::move(sphere));

	light_obj->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(), light_pos)));

	gli::texture2d light_color_tex{ gli::format::FORMAT_RGB8_UNORM_PACK8, { 1, 1 }, 1 };
	auto c = light->get_luminance();
	auto luminance = glm::length(c);
	c /= luminance;
	*reinterpret_cast<glm::u8vec3*>(light_color_tex.data()) = glm::u8vec3(c.r * 255.5f, c.g * 255.5f, c.b * 255.5f);
	
	auto layer = scene->properties().material_layers_storage().allocate_layer();
	auto mat = scene->properties().materials_storage().allocate_material(layer.get());
	mat->set_texture(std::make_unique<StE::Core::texture_2d>(light_color_tex, false));
	mat->set_emission(c * luminance);

	light_obj->set_material(mat.get());

	scene->get_object_group().add_object(light_obj);

	materials.push_back(std::move(mat));
	layers.push_back(std::move(layer));

	return light_obj;
}

void add_scene_lights(StE::Graphics::scene &scene, std::vector<std::unique_ptr<StE::Graphics::light>> &lights, std::vector<std::unique_ptr<StE::Graphics::material>> &materials, std::vector<std::unique_ptr<StE::Graphics::material_layer>> &layers) {
	std::random_device rd;
	std::mt19937 gen(rd());

	for (auto &v : { glm::vec3{ 491.2,226.1,-616.67 },
					 glm::vec3{-622.67, 645,-309 },
					 glm::vec3{  497, 645, -309},
					 glm::vec3{ 483.376,143,-222.51 },
					 glm::vec3{ 483.376,143,144.1 },
					 glm::vec3{ -242, 153,  552},
					 glm::vec3{  120, 153,  552},
					 glm::vec3{  885, 153,  552} }) {
		StE::Graphics::rgb color;
		float lums;
#ifdef STATIC_SCENE
		color = StE::Graphics::kelvin(1800);
		lums = 6500.f;
#else
		color = StE::Graphics::kelvin(std::uniform_real_distribution<>(1500,4000)(gen));
		lums = std::uniform_real_distribution<>(5000, 9000)(gen);
#endif
		auto wall_lamp = scene.properties().lights_storage().allocate_sphere_light(color, lums, v, 2.f);
		create_sphere_light_object(&scene, v, wall_lamp.get(), materials, layers);

		lights.push_back(std::move(wall_lamp));
	}
}

#ifdef _MSC_VER
int CALLBACK WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
#else
int main()
#endif
{
	/*
	 *	Create logger
	 */

	StE::log logger("Global Illumination");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";


	/*
	 *	Create GL context and window
	 */

	int w = 1920;
	int h = w * 9 / 16;
	constexpr float clip_near = 1.f;
	float fovy = glm::pi<float>() * .225f;

	GL::gl_context::context_settings settings;
	settings.vsync = false;
	settings.fs = false;
	StE::ste_engine_control ctx(std::make_unique<GL::gl_context>(settings, "Shlomi Steinberg - Global Illumination", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near);
	ctx.set_fov(fovy);

	using ResizeSignalConnectionType = StE::ste_engine_control::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);


	/*
	 *	Connect input handlers
	 */

	bool running = true;
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


	/*
	 *	Create text manager and choose default font
	 */

	auto font = StE::Text::font("Data/ArchitectsDaughter.ttf");
	StE::Resource::resource_instance<StE::Text::text_manager> text_manager(ctx, font);


	/*
	 *	Create camera
	 */

	StE::Graphics::camera camera;
	camera.set_position({ 901.4, 566.93, 112.43 });
	camera.lookat({ 771.5, 530.9, 65.6 });


	/*
	*	Create atmospheric properties
	*/
	auto atmosphere = StE::Graphics::atmospherics_earth_properties({ 0,-6.371e+6,0 });


	/*
	 *	Create and load scene object and GI renderer
	 */

	StE::Resource::resource_instance<StE::Graphics::scene> scene(ctx);
	StE::Resource::resource_instance<StE::Graphics::gi_renderer> renderer(ctx, &camera, &scene.get(), atmosphere);


	/*
	 *	Start loading resources and display loading screen
	 */

	std::vector<std::unique_ptr<StE::Graphics::light>> lights;
	std::vector<std::unique_ptr<StE::Graphics::material>> materials;
	std::vector<std::unique_ptr<StE::Graphics::material_layer>> material_layers;

	StE::task_future_collection<void> loading_futures;

	const glm::vec3 light0_pos{ -700.6, 138, -70 };
	const glm::vec3 light1_pos{ 200, 550, 170 };
	auto light0 = scene.get().properties().lights_storage().allocate_shaped_light<StE::Graphics::quad_light_onesided>(StE::Graphics::kelvin(2000), 
																													  8000.f, light0_pos);
	light0->set_points({glm::vec3{-4,-4,-4},{4,-4,4},{4,4,4},{-4,4,-4}});
	auto light1 = scene.get().properties().lights_storage().allocate_sphere_light(StE::Graphics::kelvin(7000), 
																				  20000.f, light1_pos, 5.f);
	//auto light0_obj = create_sphere_light_object(&scene.get(), light0_pos, light0.get(), materials, material_layers);
	auto light1_obj = create_sphere_light_object(&scene.get(), light1_pos, light1.get(), materials, material_layers);

	const glm::vec3 sun_direction = glm::normalize(glm::vec3{ 0.f, -1.f, 0.f });
	auto sun_light = scene.get().properties().lights_storage().allocate_directional_light(StE::Graphics::kelvin(5770), 
																						  1.88e+9f, 1496e+8f, 695e+6f, sun_direction);

	add_scene_lights(scene.get(), lights, materials, material_layers);

	std::vector<std::shared_ptr<StE::Graphics::object>> sponza_objects;
	loading_futures.insert(StE::Resource::model_factory::load_model_async(ctx,
																		 R"(Data/models/crytek-sponza/sponza.obj)",
																		 &scene.get().get_object_group(),
																		 &scene.get().properties(),
																		 2.5f,
																		 materials,
																		 material_layers,
																		 &sponza_objects));
	
	std::vector<std::unique_ptr<StE::Graphics::material>> mat_editor_materials;
	std::vector<std::unique_ptr<StE::Graphics::material_layer>> mat_editor_layers;
	std::vector<std::shared_ptr<StE::Graphics::object>> mat_editor_objects;
	loading_futures.insert(StE::Resource::model_factory::load_model_async(ctx,
																		 R"(Data/models/dragon/china_dragon.obj)",
																		 //R"(Data/models/mitsuba/mitsuba-sphere.obj)",
																		 &scene.get().get_object_group(),
																		 &scene.get().properties(),
																		 2.5f,
																		 mat_editor_materials,
																		 mat_editor_layers,
																		 &mat_editor_objects));
	loading_futures.insert(ctx.scheduler().schedule_now([&]() {
		renderer.wait();
	}));

	display_loading_screen_until(ctx, &text_manager.get(), &w, &h, [&]() -> bool {
		return running && !loading_futures.ready_all();
	});


	/*
	 *	Create debug view window and material editor
	 */

	constexpr int layers_count = 3;

	std::unique_ptr<StE::Graphics::profiler> gpu_tasks_profiler = std::make_unique<StE::Graphics::profiler>();
	renderer.get().attach_profiler(gpu_tasks_profiler.get());
	std::unique_ptr<StE::Graphics::debug_gui> debug_gui_dispatchable = std::make_unique<StE::Graphics::debug_gui>(ctx, gpu_tasks_profiler.get(), font, &camera);

	auto mat_editor_model_transform = glm::scale(glm::mat4(), glm::vec3{ 3.5f });
	mat_editor_model_transform = glm::translate(mat_editor_model_transform, glm::vec3{ .0f, -15.f, .0f });
	//auto mat_editor_model_transform = glm::translate(glm::mat4(), glm::vec3{ .0f, .0f, -50.f });
	//mat_editor_model_transform = glm::scale(mat_editor_model_transform, glm::vec3{ 65.f });
	//mat_editor_model_transform = glm::rotate(mat_editor_model_transform, glm::half_pi<float>(), glm::vec3{ .0f, 1.0f, 0.f });
	for (auto &o : mat_editor_objects)
		o->set_model_transform(glm::mat4x3(mat_editor_model_transform));
	
	std::unique_ptr<StE::Graphics::material_layer> layers[layers_count];
	layers[0] = std::move(mat_editor_layers.back());
	mat_editor_materials.back()->enable_subsurface_scattering(true);

	float dummy = .0f;

	float sun_zenith = .0f;
	float mie_absorption_coefficient = 2.2f;
	float mie_scattering_coefficient = 2e+1f;

	bool layer_enabled[3] = { true, false, false };
	StE::Graphics::rgb base_color[3];
	float roughness[3];
	float anisotropy[3];
	float metallic[3];
	float index_of_refraction[3];
	float thickness[3];
	float absorption[3];
	float phase[3];

	for (int i = 0; i < layers_count; ++i) {
		if (i > 0)
			layers[i] = scene.get().properties().material_layers_storage().allocate_layer();

		base_color[i] = { 1,1,1 };
		roughness[i] = .5f;
		anisotropy[i] = 0;
		metallic[i] = 0;
		index_of_refraction[i] = 1.5f;
		thickness[i] = 0.001f;
		absorption[i] = 1.f;
		phase[i] = .0f;
	}

	debug_gui_dispatchable->add_custom_gui([&](const glm::ivec2 &bbsize) {
		if (ImGui::Begin("Material Editor", nullptr)) {
			for (int i = 0; i < layers_count; ++i) {
				std::string layer_label = std::string("Layer ") + std::to_string(i);
				if (i != 0)
					ImGui::Checkbox(layer_label.data(), &layer_enabled[i]);
				else
					ImGui::Text(layer_label.data());

				if (layer_enabled[i]) {
					ImGui::SliderFloat((std::string("R ##value") +		" ##" + layer_label).data(), &base_color[i].R(),	 .0f, 1.f);
					ImGui::SliderFloat((std::string("G ##value") +		" ##" + layer_label).data(), &base_color[i].G(),	 .0f, 1.f);
					ImGui::SliderFloat((std::string("B ##value") +		" ##" + layer_label).data(), &base_color[i].B(),	 .0f, 1.f);
					ImGui::SliderFloat((std::string("Rghn ##value") +	" ##" + layer_label).data(), &roughness[i],			 .0f, 1.f);
					ImGui::SliderFloat((std::string("Aniso ##value") +	" ##" + layer_label).data(), &anisotropy[i],		-1.f, 1.f, "%.3f", 2.f);
					ImGui::SliderFloat((std::string("Metal ##value") +	" ##" + layer_label).data(), &metallic[i],			 .0f, 1.f);
					ImGui::SliderFloat((std::string("IOR ##value") +	" ##" + layer_label).data(), &index_of_refraction[i],1.f, 4.f, "%.5f", 3.f);
					if (i < layers_count - 1 && layer_enabled[i + 1])
						ImGui::SliderFloat((std::string("Thick ##value") + " ##" + layer_label).data(), &thickness[i], .0f, StE::Graphics::material_layer_max_thickness, "%.5f", 3.f);
					ImGui::SliderFloat((std::string("Attn ##value") +	" ##" + layer_label).data(), &absorption[i], .000001f, 50.f, "%.8f", 5.f);
					ImGui::SliderFloat((std::string("Phase ##value") +	" ##" + layer_label).data(), &phase[i], -1.f, +1.f);
				}
			}
		}

		ImGui::End();
		
		for (int i = 0; i < layers_count; ++i) {
			auto t = glm::u8vec3(base_color[i].R() * 255.5f, base_color[i].G() * 255.5f, base_color[i].B() * 255.5f);
			if (layers[i]->get_albedo() != base_color[i])
				layers[i]->set_albedo(base_color[i]);
			if (layers[i]->get_roughness() != roughness[i])
				layers[i]->set_roughness(roughness[i]);
			if (layers[i]->get_anisotropy() != anisotropy[i])
				layers[i]->set_anisotropy(anisotropy[i]);
			if (layers[i]->get_metallic() != metallic[i])
				layers[i]->set_metallic(metallic[i]);
			if (layers[i]->get_index_of_refraction() != index_of_refraction[i])
				layers[i]->set_index_of_refraction(index_of_refraction[i]);
			if (layers[i]->get_layer_thickness() != thickness[i])
				layers[i]->set_layer_thickness(thickness[i]);
			if (layers[i]->get_attenuation_coefficient().x != absorption[i])
				layers[i]->set_attenuation_coefficient(glm::vec3{ absorption[i] });
			if (layers[i]->get_scattering_phase_parameter() != phase[i])
				layers[i]->set_scattering_phase_parameter(phase[i]);

			if (i != 0) {
				bool enabled = layers[i - 1]->get_next_layer() != nullptr;
				if (layer_enabled[i] != enabled)
					layers[i - 1]->set_next_layer(layer_enabled[i] ? layers[i].get() : nullptr);
			}
		}

		if (ImGui::Begin("Atmosphere", nullptr)) {
			ImGui::SliderFloat((std::string("Sun zenith angle ##value")).data(), &sun_zenith, .0f, 2 * glm::pi<float>());
			ImGui::SliderFloat((std::string("Mie scattering coefficient (10^-8) ##value##mie1")).data(), &mie_scattering_coefficient, .0f, 100.f, "%.5f", 3.f);
			ImGui::SliderFloat((std::string("Mie absorption coefficient (10^-8) ##value##mie2")).data(), &mie_absorption_coefficient, .0f, 100.f, "%.5f", 3.f);
			ImGui::SliderFloat((std::string("Debug dummy variable")).data(), &dummy, .0f, 1.f);
		}

		ImGui::End();

		atmosphere.mie_absorption_coefficient = static_cast<double>(mie_absorption_coefficient) * 1e-8;
		atmosphere.mie_scattering_coefficient = static_cast<double>(mie_scattering_coefficient) * 1e-8;
		renderer.get().update_atmospherics_properties(atmosphere);

		renderer.get().get_composer_program().set_uniform("dummy", dummy);
	});


	/*
	 *	Configure GI renderer
	 */
	renderer.get().set_aperture_parameters(35e-3, 25e-3);


	/*
	 *	Switch to GI renderer and start render loop
	 */

	ctx.set_renderer(&renderer.get());

	auto footer_text = text_manager.get().create_renderer();
	auto footer_text_task = StE::Graphics::make_gpu_task("footer_text", footer_text.get(), nullptr);

#ifndef STATIC_SCENE
	renderer.get().add_gui_task(footer_text_task);
#endif
	renderer.get().add_gui_task(StE::Graphics::make_gpu_task("debug_gui", debug_gui_dispatchable.get(), nullptr));

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
			bool rotate_camera = mouse_down;

			auto pp = ctx.get_pointer_position();
			if (mouse_down && !debug_gui_dispatchable->is_gui_active()) {
				auto diff_v = static_cast<glm::vec2>(last_pointer_pos - pp) * time_delta * rotation_factor;
				camera.pitch_and_yaw(-diff_v.y, diff_v.x);
			}
			last_pointer_pos = pp;
		}

#ifdef STATIC_SCENE
		glm::vec3 lp = light0_pos;
		glm::vec3 sun_dir = sun_direction;
#else
		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;

		glm::vec3 sun_dir = glm::normalize(glm::vec3{ glm::sin(sun_zenith + glm::pi<float>()), 
													  -glm::cos(sun_zenith + glm::pi<float>()), 
													  .15f});
#endif
		light0->set_position(lp);
		//light0_obj->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(), lp)));
		sun_light->set_direction(sun_dir);

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
