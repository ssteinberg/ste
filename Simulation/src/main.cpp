
#include <stdafx.hpp>
#include <ste_engine.hpp>
#include <presentation_engine.hpp>
#include <presentation_frame_time_predictor.hpp>

#include <keyboard.hpp>
#include <pointer.hpp>

#include <cmd_pipeline_barrier.hpp>

#include <model_factory.hpp>
#include <surface.hpp>
#include <surface_factory.hpp>
#include <text_manager.hpp>
#include <text_fragment.hpp>
#include <attrib.hpp>

#include <rendering_presentation_system.hpp>

#include <scene.hpp>
#include <atmospherics_properties.hpp>
#include <primary_renderer.hpp>
#include <sphere.hpp>
#include <quad_light.hpp>

#include <camera.hpp>
#include <camera_projection_reversed_infinite_perspective.hpp>

#include <random>

//#include <imgui/imgui.h>
//#include <debug_gui.hpp>

//#define STATIC_SCENE

using namespace ste;

class loading_photo_fragment : public gl::fragment_graphics<loading_photo_fragment> {
	using Base = gl::fragment_graphics<loading_photo_fragment>;

	gl::texture<gl::image_type::image_2d> photo;
	gl::task<gl::cmd_draw> draw_task;

public:
	loading_photo_fragment(gl::rendering_presentation_system &rs,
						   gl::framebuffer_layout &&fb_layout)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   std::move(fb_layout),
			   "fullscreen_triangle.vert", "loading.frag"),
		photo(resource::surface_factory::image_from_surface_2d<gl::format::r8g8b8a8_unorm>(rs.get_creating_context(),
																						   resource::surface_convert::convert_2d<gl::format::r8g8b8a8_srgb>(resource::surface_io::load_surface_2d("Data/loading.jpeg", true)),
																						   gl::image_usage::sampled,
																						   gl::image_layout::shader_read_only_optimal,
																						   "loading_background_photo"))
	{
		pipeline()["sam"] = gl::bind(gl::pipeline::combined_image_sampler(photo,
																		  rs.get_creating_context().device().common_samplers_collection().linear_clamp_sampler()));

		draw_task.attach_pipeline(pipeline());
	}

	static lib::string name() { return "loading_photo_fragment"; }

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline().attach_framebuffer(fb);
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

class loading_renderer : public gl::rendering_presentation_system {
	using Base = gl::rendering_presentation_system;

private:
	std::reference_wrapper<gl::presentation_engine> presentation;

	loading_photo_fragment photo_fragment;
	text::text_fragment title_text_frag;
	text::text_fragment footer_text_frag;

	lib::vector<gl::framebuffer> swap_chain_clear_framebuffers;

	gl::ste_device::queues_and_surface_recreate_signal_type::connection_type resize_signal_connection;

private:
	static auto create_fb_clear_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::clear_store(ctx.device().get_surface().surface_format(),
									   gl::image_layout::color_attachment_optimal);
		return fb_layout;
	}
	static auto create_fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::load_store(ctx.device().get_surface().surface_format(),
									  gl::image_layout::color_attachment_optimal,
									  gl::image_layout::color_attachment_optimal);
		return fb_layout;
	}

	void create_swap_chain_clear_framebuffers() {
		auto surface_extent = device().get_surface().extent();

		lib::vector<gl::framebuffer> v;
		for (auto &swap_image : device().get_surface().get_swap_chain_images()) {
			gl::framebuffer fb(get_creating_context(),
							   "swap_chain_clear_framebuffer",
							   create_fb_clear_layout(get_creating_context()),
							   surface_extent);
			fb[0] = gl::framebuffer_attachment(swap_image.view, glm::vec4(.0f));

			v.push_back(std::move(fb));
		}

		swap_chain_clear_framebuffers = std::move(v);
	}

public:
	loading_renderer(const ste_context &ctx,
					 gl::presentation_engine &presentation,
					 text::text_manager &tm)
		: Base(ctx,
			   create_fb_layout(ctx)),
		presentation(presentation),
		photo_fragment(*this, create_fb_clear_layout(ctx)),
		title_text_frag(tm.create_fragment()),
		footer_text_frag(tm.create_fragment())
	{
		create_swap_chain_clear_framebuffers();
	}
	~loading_renderer() noexcept {}

	void render(gl::command_recorder &recorder) override final {
		{
			using namespace text::attributes;

			const text::attributed_wstring str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
														azure(large(L"Loading...\n")) +
														orange(regular(L"By Shlomi Steinberg")));
			auto surface_extent = device().get_surface().extent();

			auto total_vram = get_creating_context().device_memory_allocator().get_total_device_memory() / 1024 / 1024;
			auto commited_vram = get_creating_context().device_memory_allocator().get_total_commited_memory() / 1024 / 1024;
			auto allocated_vram = get_creating_context().device_memory_allocator().get_total_allocated_memory() / 1024 / 1024;

			auto workers_active = get_creating_context().engine().task_scheduler().get_thread_pool()->get_active_workers_count();
			auto workers_sleep = get_creating_context().engine().task_scheduler().get_thread_pool()->get_sleeping_workers_count();
			auto pending_requests = get_creating_context().engine().task_scheduler().get_thread_pool()->get_pending_requests_count();

			title_text_frag.update_text(get_creating_context(),
										recorder, { surface_extent.x / 2, surface_extent.y / 2 + 100 }, str);
			footer_text_frag.update_text(get_creating_context(), 
										 recorder, { 10, 50 },
										 line_height(32)(vsmall(b(L"VRAM ") +
																b(medium_violet_red(lib::to_wstring(allocated_vram)) + L" / " +
																  purple(lib::to_wstring(commited_vram)) + L" / " +
																  stroke(blue, 1)(sky_blue(lib::to_wstring(total_vram))) + L" MB")) + L"\n" +
														 vsmall(b(L"Thread pool workers: ") +
																olive(lib::to_wstring(workers_active)) + L" busy, " +
																olive(lib::to_wstring(workers_sleep)) + L" sleeping | " +
																orange(lib::to_wstring(pending_requests) + L" pending requests"))));
		}

		recorder
			<< photo_fragment

			// Render text
			<< title_text_frag
			<< footer_text_frag;
	}

	void present() override final {
		auto selector = gl::make_queue_selector(gl::ste_queue_type::primary_queue);

		// Acquire presentation comand batch
		auto batch = presentation.get().allocate_presentation_command_batch(selector);

		// Record and submit a batch
		device().enqueue(selector, [this, batch = std::move(batch)]() mutable {
			auto& command_buffer = batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();
				auto &fb = swap_chain_framebuffer(batch->presentation_image_index());
				auto &swapchain_image = swap_chain_image(batch->presentation_image_index()).image;

				title_text_frag.manager().attach_framebuffer(fb);
				photo_fragment.attach_framebuffer(swap_chain_clear_framebuffers[batch->presentation_image_index()]);

				render(recorder);
				recorder
					// Prepare framebuffer for presentation
					<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::color_attachment_output,
																	 gl::pipeline_stage::bottom_of_pipe,
																	 gl::image_layout_transform_barrier(swapchain_image,
																										gl::image_layout::color_attachment_optimal,
																										gl::image_layout::present_src_khr)));
			}

			// Submit command buffer and present
			presentation.get().submit_and_present(std::move(batch));
		});
	}
};

class gi_renderer : public gl::rendering_presentation_system {
	using Base = gl::rendering_presentation_system;

private:
	std::reference_wrapper<gl::presentation_engine> presentation;
	graphics::primary_renderer r;
	
	gl::ste_device::queues_and_surface_recreate_signal_type::connection_type resize_signal_connection;

private:
	static auto fb_layout(const ste_context &ctx) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::ignore_store(ctx.device().get_surface().surface_format(),
										gl::image_layout::color_attachment_optimal);
		return fb_layout;
	}

public:
	gi_renderer(const ste_context &ctx,
				gl::presentation_engine &presentation,
				const graphics::primary_renderer::camera_t *cam,
				graphics::scene *s,
				const graphics::atmospherics_properties<double> &atmospherics_prop)
		: Base(ctx,
			   fb_layout(ctx)),
		presentation(presentation),
		r(ctx, fb_layout(ctx), cam, s, atmospherics_prop)
	{}
	~gi_renderer() noexcept {}

	auto &renderer() { return r; }
	auto &renderer() const { return r; }

	void render(gl::command_recorder &recorder) override final {
		auto total_vram = get_creating_context().device_memory_allocator().get_total_device_memory() / 1024 / 1024;
		auto commited_vram = get_creating_context().device_memory_allocator().get_total_commited_memory() / 1024 / 1024;
		auto allocated_vram = get_creating_context().device_memory_allocator().get_total_allocated_memory() / 1024 / 1024;

		r.render(recorder);
	}

	void present() override final {
		auto selector = gl::make_queue_selector(gl::ste_queue_type::primary_queue);

		// Acquire presentation comand batch
		auto batch = presentation.get().allocate_presentation_command_batch(selector);

		auto debugz_fence = batch->get_fence_ptr();

		// Record and submit a batch
		device().enqueue(selector, [this, batch = std::move(batch)]() mutable {
			auto& command_buffer = batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();
				auto &fb = swap_chain_framebuffer(batch->presentation_image_index());
				auto &swapchain_image = swap_chain_image(batch->presentation_image_index()).image;

				r.attach_framebuffer(fb);

				render(recorder);
				recorder
					// Prepare framebuffer for presentation
					<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::color_attachment_output,
																	 gl::pipeline_stage::bottom_of_pipe,
																	 gl::image_layout_transform_barrier(swapchain_image,
																										gl::image_layout::color_attachment_optimal,
																										gl::image_layout::present_src_khr)));
			}

			// Submit command buffer and present
			presentation.get().submit_and_present(std::move(batch));
		});
	}
};

auto requested_device_features() {
	VkPhysicalDeviceFeatures requested_features;
	memset(&requested_features, 0, sizeof(requested_features));
	requested_features.drawIndirectFirstInstance = VK_TRUE;
	requested_features.fragmentStoresAndAtomics = VK_TRUE;
	requested_features.geometryShader = VK_TRUE;
	requested_features.imageCubeArray = VK_TRUE;
	requested_features.multiDrawIndirect = VK_TRUE;
	requested_features.samplerAnisotropy = VK_TRUE;
	requested_features.shaderStorageImageExtendedFormats = VK_TRUE;
	requested_features.shaderImageGatherExtended = VK_TRUE;
	requested_features.sparseBinding = VK_TRUE;
	requested_features.sparseResidencyBuffer = VK_TRUE;
	requested_features.tessellationShader = VK_TRUE;

	return requested_features;
}


void display_loading_screen_until(ste_context &ctx,
								  gl::presentation_engine &presentation,
								  text::text_manager &text_manager,
								  const ste_window &window,
								  std::function<bool()> &&lambda) {
	loading_renderer r(ctx, presentation, text_manager);

	for (;;) {
		ctx.tick();
		ste_window::poll_events();

		if (!lambda())
			break;

		r.present();
	}

	ctx.device().wait_idle();
}

auto create_light_mesh(const ste_context &ctx,
					   graphics::scene *scene,
					   const graphics::rgb &color,
					   float intensity,
					   const glm::vec3 &light_pos,
					   lib::unique_ptr<graphics::mesh_generic> &&mesh,
					   lib::vector<lib::unique_ptr<graphics::material>> &materials,
					   lib::vector<lib::unique_ptr<graphics::material_layer>> &layers) {
	auto light_obj = lib::allocate_shared<graphics::object>(std::move(mesh));

	light_obj->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(1.f), light_pos)));

	resource::surface_2d<gl::format::r8g8b8a8_unorm> light_color_tex{ { 1, 1 } };
	auto c = glm::clamp(static_cast<glm::vec3>(color) / color.luminance(), glm::vec3(.0f), glm::vec3(1.f));
	light_color_tex[0][0].r() = static_cast<std::uint8_t>(c.r * 255.5f);
	light_color_tex[0][0].g() = static_cast<std::uint8_t>(c.g * 255.5f);
	light_color_tex[0][0].b() = static_cast<std::uint8_t>(c.b * 255.5f);

	auto layer = scene->properties().material_layers_storage().allocate_layer();
	auto mat = scene->properties().materials_storage().allocate_material(ctx,
																		 layer.get());
	auto tex = scene->properties().material_textures_storage().allocate_texture(resource::surface_factory::image_from_surface_2d<gl::format::r8_unorm>(ctx,
																																					   std::move(light_color_tex),
																																					   gl::image_usage::sampled,
																																					   gl::image_layout::shader_read_only_optimal,
																																					   "light mesh color texture"));

	mat->set_texture(tex);
	mat->set_emission(static_cast<glm::vec3>(color) * intensity);

	light_obj->set_material(mat.get());

	// Create a batch
	auto fence = ctx.device().submit_onetime_batch(gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::data_transfer_sparse_queue),
												   [=, &ctx](gl::command_recorder &recorder) {
		scene->get_object_group().add_object(ctx, recorder, light_obj);
	});

	materials.push_back(std::move(mat));
	layers.push_back(std::move(layer));

	return light_obj;
}

auto create_sphere_light_object(const ste_context &ctx,
								graphics::scene *scene,
								const graphics::rgb &color,
								float intensity,
								float radius,
								const glm::vec3 &light_pos,
								lib::vector<lib::unique_ptr<graphics::material>> &materials,
								lib::vector<lib::unique_ptr<graphics::material_layer>> &layers) {
	auto light = scene->properties().lights_storage().allocate_sphere_light(color, intensity, light_pos, radius);

	lib::unique_ptr<graphics::sphere> sphere = lib::allocate_unique<graphics::sphere>(10, 10);
	(*sphere) *= light->get_radius();

	auto light_obj = create_light_mesh(ctx, scene, color, intensity, light_pos, std::move(sphere), materials, layers);

	return std::make_pair(std::move(light), light_obj);
}

auto create_quad_light_object(const ste_context &ctx,
							  graphics::scene *scene,
							  const graphics::rgb &color,
							  float intensity,
							  const glm::vec3 &light_pos,
							  const glm::vec3 &n, const glm::vec3 &t,
							  const glm::vec3 points[4],
							  lib::vector<lib::unique_ptr<graphics::material>> &materials,
							  lib::vector<lib::unique_ptr<graphics::material_layer>> &layers) {
	auto light = scene->properties().lights_storage().allocate_shaped_light<graphics::quad_light_onesided>(color, intensity, light_pos);
	ctx.device().submit_onetime_batch(gl::ste_queue_selector<gl::ste_queue_selector_policy_flexible>(gl::ste_queue_type::data_transfer_sparse_queue),
									  [&](gl::command_recorder &recorder) {
		light->set_points(ctx, 
						  recorder,
						  points);
	});

	auto quad = lib::allocate_unique<graphics::mesh<graphics::mesh_subdivion_mode::Triangles>>();
	std::uint32_t ind[6] = { 0,1,2,0,2,3 };
	graphics::object_vertex_data vertices[4];
	vertices[0].p() = points[0]; vertices[1].p() = points[1]; vertices[2].p() = points[2]; vertices[3].p() = points[3];
	const glm::vec3 b = glm::cross(t, n);
	for (auto &v : vertices)
		v.tangent_frame_from_tbn(t, b, n);
	quad->set_vertices(vertices, 4);
	quad->set_indices(ind, 6);

	auto light_obj = create_light_mesh(ctx, scene, color, intensity, light_pos, std::move(quad), materials, layers);

	return std::make_pair(std::move(light), light_obj);
}

void add_scene_lights(const ste_context &ctx,
					  graphics::scene &scene,
					  lib::vector<lib::unique_ptr<graphics::light>> &lights,
					  lib::vector<lib::unique_ptr<graphics::material>> &materials,
					  lib::vector<lib::unique_ptr<graphics::material_layer>> &layers) {
	std::random_device rd;
	std::mt19937 gen(rd());

	for (auto &v : { glm::vec3{ 491.2,226.1,-616.67 },
					 glm::vec3{ 483.376,143,-222.51 },
					 glm::vec3{ 483.376,143,144.1 },
					 glm::vec3{ -242, 153,  552},
					 glm::vec3{  885, 153,  552} }) {
#ifdef STATIC_SCENE
		const graphics::rgb color = graphics::kelvin(1800);
		const float lums = 4500.f;
#else
		const graphics::rgb color = graphics::kelvin(std::uniform_real_distribution<float>(1300, 4500)(gen));
		const float lums = std::uniform_real_distribution<float>(3000, 4000)(gen) / color.luminance();
#endif
		auto wall_lamp = create_sphere_light_object(ctx, &scene, color, lums, 1.f, v, materials, layers);

		lights.push_back(std::move(wall_lamp.first));
	}

	glm::vec3 points[4] = { { -18,18,0 },{ 18,18,0 },{ 18,-18,0 },{ -18,-18,0 } };
	auto lamp = create_quad_light_object(ctx, &scene, graphics::kelvin(12000), 3000, glm::vec3{ 120, 153, 565 },
										 glm::vec3{ 0,0,-1 }, glm::vec3{ 1,0,0 }, points, materials, layers);

	lights.push_back(std::move(lamp.first));
}

void load_scene(ste_context &ctx,
				graphics::scene &scene,
				gl::presentation_engine &presentation,
				text::text_manager &text_manager,
				const ste_window &window,
				task_future_collection<void> &&loading_futures,
				lib::vector<lib::unique_ptr<graphics::light>> &lights,
				lib::vector<lib::unique_ptr<graphics::material>> &materials,
				lib::vector<lib::unique_ptr<graphics::material_layer>> &material_layers,
				lib::vector<lib::shared_ptr<graphics::object>> &scene_objects,
				lib::vector<lib::unique_ptr<graphics::material>> &mat_editor_materials,
				lib::vector<lib::unique_ptr<graphics::material_layer>> &mat_editor_layers,
				lib::vector<lib::shared_ptr<graphics::object>> &mat_editor_objects) {
	// Create all scene lights
	loading_futures.insert(ctx.engine().task_scheduler().schedule_now([&]() {
		add_scene_lights(ctx, scene, lights, materials, material_layers);
	}));

	// Load models
	loading_futures.insert(resource::model_factory::load_model_async(ctx,
																	 R"(Data/models/crytek-sponza/sponza.obj)",
																	 &scene.get_object_group(),
																	 &scene.properties(),
																	 2.5f,
																	 materials,
																	 material_layers,
																	 &scene_objects));
	loading_futures.insert(resource::model_factory::load_model_async(ctx,
																	 R"(Data/models/dragon/china_dragon.obj)",
																	 //R"(Data/models/mitsuba/mitsuba-sphere.obj)",
																	 &scene.get_object_group(),
																	 &scene.properties(),
																	 2.5f,
																	 mat_editor_materials,
																	 mat_editor_layers,
																	 &mat_editor_objects));

	display_loading_screen_until(ctx, presentation, text_manager, window, [&]() -> bool {
		return !loading_futures.ready_all();
	});
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
	log::log logger("Global Illumination");
	logger.redirect_std_outputs();
	log::ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	/*
	*	Create StE engine instance
	*/
	ste_engine engine;


	/*
	*	Create window
	*/
	ste_window window("StE - Simulation", { 1920, 1080 });
	auto& window_signals = window.get_signals();


	/*
	*	Create gl context and query physical devices
	*/
	gl::ste_gl_context_creation_parameters gl_params;
	gl_params.client_name = "Simulation";
	gl_params.client_version = 1;
	gl_params.debug_context = false;
#ifdef DEBUG
	gl_params.debug_context = true;
#endif

	gl::ste_gl_context gl_ctx(gl_params);

	auto features = requested_device_features();
	auto available_devices = gl_ctx.enumerate_physical_devices(features, 4000ul * 1024 * 1024);
	auto physical_device = available_devices[0];


	/*
	*	Select a physical device, create a presentation device and a context
	*/
	gl::ste_gl_device_creation_parameters device_params;
	device_params.physical_device = physical_device;
	device_params.requested_device_features = features;
	device_params.additional_device_extensions = { "VK_KHR_shader_draw_parameters" };
	device_params.allow_markers = false;
	device_params.presentation_surface_parameters.vsync = gl::ste_presentation_device_vsync::mailbox;
	device_params.presentation_surface_parameters.simultaneous_presentation_frames = 3;

	ste_context::gl_device_t device(device_params,
									gl::ste_device_queues_protocol::queue_descriptors_for_physical_device(physical_device),
									engine,
									gl_ctx,
									window);
	ste_context ctx(engine, gl_ctx, device);


	/*
	*	Create the presentation engine
	*/
	gl::presentation_engine presentation(device);
	gl::presentation_frame_time_predictor frame_time_predictor;


	/*
	*	Text renderer
	*/
	const auto text_manager_font = text::font("Data/ArchitectsDaughter.ttf");
	text::text_manager text_manager(ctx, text_manager_font);


	/*
	*	Create camera
	*/
	constexpr float clip_near = .1f;
	const float fovy = glm::pi<float>() * .225f;
	const float aspect = static_cast<float>(ctx.device().get_surface().extent().x) / static_cast<float>(ctx.device().get_surface().extent().y);
	graphics::camera<float, graphics::camera_projection_reversed_infinite_perspective> camera(graphics::camera_projection_reversed_infinite_perspective<float>(fovy, aspect, clip_near));

	// Set initial camera position
	camera.set_position({ 901.4, 566.93, 112.43 });
	camera.lookat({ 771.5, 530.9, 65.6 });

	// Attach a connection to swapchain's surface resize signal
	auto resize_signal_connection = make_connection(ctx.device().get_queues_and_surface_recreate_signal(), [&](auto) {
		// Update camera's projection model on swapchain resize
		const float new_aspect = static_cast<float>(ctx.device().get_surface().extent().x) / static_cast<float>(ctx.device().get_surface().extent().y);
		camera.set_projection_model(graphics::camera_projection_reversed_infinite_perspective<float>(fovy, new_aspect, clip_near));
	});


	/*
	*	Create atmospheric properties
	*/
	auto atmosphere = graphics::atmospherics_earth_properties({ 0,-6.371e+6,0 });


	/*
	*	Create scene and primary render
	*/
	graphics::scene scene(ctx);
	lib::unique_ptr<gi_renderer> presenter;
	auto renderer_loader_task_future = ctx.engine().task_scheduler().schedule_now([&]() {
		// Create renderer
		presenter = lib::allocate_unique<gi_renderer>(ctx,
													  presentation,
													  &camera,
													  &scene,
													  atmosphere);
	});


	/*
	 *	load scene resources and display loading screen
	 */

	 // All scene resources
	lib::vector<lib::unique_ptr<graphics::light>> lights;
	lib::vector<lib::unique_ptr<graphics::material>> materials;
	lib::vector<lib::unique_ptr<graphics::material_layer>> material_layers;
	lib::vector<lib::shared_ptr<graphics::object>> scene_objects;
	lib::vector<lib::unique_ptr<graphics::material>> mat_editor_materials;
	lib::vector<lib::unique_ptr<graphics::material_layer>> mat_editor_layers;
	lib::vector<lib::shared_ptr<graphics::object>> mat_editor_objects;

	// Load
	{
		task_future_collection<void> loading_futures;
		loading_futures.insert(std::move(renderer_loader_task_future));
		load_scene(ctx, scene, presentation, text_manager, window,
				   std::move(loading_futures),
				   lights,
				   materials,
				   material_layers,
				   scene_objects,
				   mat_editor_materials,
				   mat_editor_layers,
				   mat_editor_objects);

		if (window.should_close()) {
			return 0;
		}
	}

	// Configure
	presenter->renderer().set_aperture_parameters(8e-3f, 25e-3f);

	const glm::vec3 light0_pos{ -700.6, 138, -70 };
	const glm::vec3 light1_pos{ 200, 550, 170 };
	auto light0 = create_sphere_light_object(ctx, &scene, graphics::kelvin(2000), 2000.f, 2.f, light0_pos, materials, material_layers);
	auto light1 = create_sphere_light_object(ctx, &scene, graphics::kelvin(7000), 8000.f, 4.f, light1_pos, materials, material_layers);

	const glm::vec3 sun_direction = glm::normalize(glm::vec3{ 0.f, -1.f, 0.f });
	auto sun_light = scene.properties().lights_storage().allocate_directional_light(graphics::kelvin(5770),
																					1.88e+9f, 1496e+8f, 695e+6f, sun_direction);


	/*
	 *	GUI
	 */
	float sun_zenith = .0f;


	/*
	 *	Input handlers
	 */

	bool running = true;
	bool mouse_down = false;
	glm::vec2 pointer_pos = { .0, .0 }, last_pointer_pos;

	auto keyboard_connection = ste::make_connection(window.get_signals().signal_keyboard(),
													[&](auto key, auto scanline, auto status, auto mods) {
		if (status != hid::status::down)
			return;

		if (key == hid::key::KeyESCAPE)
			running = false;
		//		if (key == hid::key::KeyPRINT_SCREEN || key == hid::key::KeyF12)
		//			capture_screenshot();
	});
	auto pointer_button_connection = ste::make_connection(window.get_signals().signal_pointer_button(),
														  [&](auto b, auto status, auto mods) {
		mouse_down = b == hid::button::Left && status == hid::status::down;
	});
	auto pointer_movement_connection = ste::make_connection(window.get_signals().signal_pointer_movement(),
															[&](auto pos) {
		pointer_pos = glm::vec2(pos);
	});


	/*
	*	Main loop
	*/

	float time_elapsed = .0f;
	for (;;) {
		ctx.tick();
		ste_window::poll_events();

		if (window.should_close() || !running) {
			break;
		}

		// Calculate predicted next frame time
		frame_time_predictor.update(presentation.get_frame_time());
		float frame_time_ms = frame_time_predictor.predicted_value();
		window.set_title(lib::to_string(frame_time_ms).c_str());

		if (window.is_window_focused()) {
			// Handle movement input
			constexpr float movement_factor = .4f;
			if (hid::keyboard::key_status(window, hid::key::KeyW) == hid::status::down)
				camera.step_forward(frame_time_ms * movement_factor);
			if (hid::keyboard::key_status(window, hid::key::KeyS) == hid::status::down)
				camera.step_backward(frame_time_ms * movement_factor);
			if (hid::keyboard::key_status(window, hid::key::KeyA) == hid::status::down)
				camera.step_left(frame_time_ms * movement_factor);
			if (hid::keyboard::key_status(window, hid::key::KeyD) == hid::status::down)
				camera.step_right(frame_time_ms * movement_factor);

			// Handle camera rotation input
			constexpr float rotation_factor = .0002f;
			bool rotate_camera = mouse_down;
			if (mouse_down/* && !debug_gui_dispatchable->is_gui_active()*/) {
				const auto diff_v = static_cast<glm::vec2>(last_pointer_pos - pointer_pos) * frame_time_ms * rotation_factor;
				camera.pitch_and_yaw(diff_v.y, diff_v.x);
			}
			last_pointer_pos = pointer_pos;
		}

		// Update scene objects
#ifdef STATIC_SCENE
		glm::vec3 lp = light0_pos;
		glm::vec3 sun_dir = sun_direction;
#else
		const float angle = time_elapsed * glm::pi<float>() *.00025f;
		const glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;

		const glm::vec3 sun_dir = glm::normalize(glm::vec3{ glm::sin(sun_zenith + glm::pi<float>()),
															-glm::cos(sun_zenith + glm::pi<float>()),
															.15f });

		light0.first->set_position(lp);
		light0.second->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(1.f), lp)));
		sun_light->set_direction(sun_dir);

		time_elapsed += frame_time_ms;
#endif

		// Present
		presenter->present();
	}

	device.wait_idle();

	return 0;
}

//
//#ifdef _MSC_VER
//int CALLBACK WinMain(HINSTANCE hInstance,
//					 HINSTANCE hPrevInstance,
//					 LPSTR     lpCmdLine,
//					 int       nCmdShow)
//#else
//int main()
//#endif
//{
	 //
	 //
	 //	/*
	 //	 *	Connect input handlers
	 //	 */
	 //
	 //	bool running = true;
	 //	bool mouse_down = false;
	 //	auto keyboard_listner = std::make_shared<decltype(ctx)::hid_keyboard_signal_type::connection_type>(
	 //		[&](hid::keyboard::K key, int scanline, hid::Status status, hid::ModifierBits mods) {
	 //		using namespace hid;
	 //		auto time_delta = ctx.time_per_frame().count();
	 //
	 //		if (status != Status::KeyDown)
	 //			return;
	 //
	 //		if (key == keyboard::K::KeyESCAPE)
	 //			running = false;
	 //		if (key == keyboard::K::KeyPRINT_SCREEN || key == keyboard::K::KeyF12)
	 //			ctx.capture_screenshot();
	 //	});
	 //	auto pointer_button_listner = std::make_shared<decltype(ctx)::hid_pointer_button_signal_type::connection_type>(
	 //		[&](hid::pointer::B b, hid::Status status, hid::ModifierBits mods) {
	 //		using namespace hid;
	 //
	 //		mouse_down = b == pointer::B::Left && status == Status::KeyDown;
	 //	});
	 //	ctx.hid_signal_keyboard().connect(keyboard_listner);
	 //	ctx.hid_signal_pointer_button().connect(pointer_button_listner);
	 //
	 //
	 //
	 //	/*
	 //	 *	Create debug view window and material editor
	 //	 */
	 //
	 //	constexpr int layers_count = 3;
	 //
	 //	std::unique_ptr<graphics::profiler> gpu_tasks_profiler = std::make_unique<graphics::profiler>();
	 //	renderer.get().attach_profiler(gpu_tasks_profiler.get());
	 //	std::unique_ptr<graphics::debug_gui> debug_gui_dispatchable = std::make_unique<graphics::debug_gui>(ctx, gpu_tasks_profiler.get(), font, &camera);
	 //
	 //	auto mat_editor_model_transform = glm::scale(glm::mat4(1.f), glm::vec3{ 3.5f });
	 //	mat_editor_model_transform = glm::translate(mat_editor_model_transform, glm::vec3{ .0f, -15.f, .0f });
	 //	//auto mat_editor_model_transform = glm::translate(glm::mat4(1.f), glm::vec3{ .0f, .0f, -50.f });
	 //	//mat_editor_model_transform = glm::scale(mat_editor_model_transform, glm::vec3{ 65.f });
	 //	//mat_editor_model_transform = glm::rotate(mat_editor_model_transform, glm::half_pi<float>(), glm::vec3{ .0f, 1.0f, 0.f });
	 //	for (auto &o : mat_editor_objects)
	 //		o->set_model_transform(glm::mat4x3(mat_editor_model_transform));
	 //	
	 //	std::unique_ptr<graphics::material_layer> layers[layers_count];
	 //	layers[0] = std::move(mat_editor_layers.back());
	 //	mat_editor_materials.back()->enable_subsurface_scattering(true);
	 //
	 //	float dummy = .0f;
	 //
	 //	float sun_zenith = .0f;
	 //	float mie_absorption_coefficient = 2.5f;
	 //	float mie_scattering_coefficient = 1.5e+1f;
	 //
	 //	bool layer_enabled[3] = { true, false, false };
	 //	graphics::rgb base_color[3];
	 //	float roughness[3];
	 ////	float anisotropy[3];
	 //	float metallic[3];
	 //	float index_of_refraction[3];
	 //	float thickness[3];
	 //	float absorption[3];
	 //	float phase[3];
	 //
	 //	for (int i = 0; i < layers_count; ++i) {
	 //		if (i > 0)
	 //			layers[i] = scene.get().properties().material_layers_storage().allocate_layer();
	 //
	 //		base_color[i] = { 1,1,1 };
	 //		roughness[i] = .5f;
	 ////		anisotropy[i] = 0;
	 //		metallic[i] = 0;
	 //		index_of_refraction[i] = 1.5f;
	 //		thickness[i] = 0.001f;
	 //		absorption[i] = 1.f;
	 //		phase[i] = .0f;
	 //	}
	 //
	 //	debug_gui_dispatchable->add_custom_gui([&](const glm::ivec2 &bbsize) {
	 //		if (ImGui::Begin("Material Editor", nullptr)) {
	 //			for (int i = 0; i < layers_count; ++i) {
	 //				std::string layer_label = std::string("Layer ") + std::to_string(i);
	 //				if (i != 0)
	 //					ImGui::Checkbox(layer_label.data(), &layer_enabled[i]);
	 //				else
	 //					ImGui::text(layer_label.data());
	 //
	 //				if (layer_enabled[i]) {
	 //					ImGui::SliderFloat((std::string("R ##value") +		" ##" + layer_label).data(), &base_color[i].R(),	 .0f, 1.f);
	 //					ImGui::SliderFloat((std::string("G ##value") +		" ##" + layer_label).data(), &base_color[i].G(),	 .0f, 1.f);
	 //					ImGui::SliderFloat((std::string("B ##value") +		" ##" + layer_label).data(), &base_color[i].B(),	 .0f, 1.f);
	 //					ImGui::SliderFloat((std::string("Rghn ##value") +	" ##" + layer_label).data(), &roughness[i],			 .0f, 1.f);
	 ////					ImGui::SliderFloat((std::string("Aniso ##value") +	" ##" + layer_label).data(), &anisotropy[i],		-1.f, 1.f, "%.3f", 2.f);
	 //					ImGui::SliderFloat((std::string("Metal ##value") +	" ##" + layer_label).data(), &metallic[i],			 .0f, 1.f);
	 //					ImGui::SliderFloat((std::string("IOR ##value") +	" ##" + layer_label).data(), &index_of_refraction[i],1.f, 4.f, "%.5f", 3.f);
	 //					if (i < layers_count - 1 && layer_enabled[i + 1])
	 //						ImGui::SliderFloat((std::string("Thick ##value") + " ##" + layer_label).data(), &thickness[i], .0f, graphics::material_layer_max_thickness, "%.5f", 3.f);
	 //					ImGui::SliderFloat((std::string("Attn ##value") +	" ##" + layer_label).data(), &absorption[i], .000001f, 50.f, "%.8f", 5.f);
	 //					ImGui::SliderFloat((std::string("Phase ##value") +	" ##" + layer_label).data(), &phase[i], -1.f, +1.f);
	 //				}
	 //			}
	 //		}
	 //
	 //		ImGui::End();
	 //		
	 //		for (int i = 0; i < layers_count; ++i) {
	 //			auto t = glm::u8vec3(base_color[i].R() * 255.5f, base_color[i].G() * 255.5f, base_color[i].B() * 255.5f);
	 //			if (layers[i]->get_albedo() != base_color[i])
	 //				layers[i]->set_albedo(base_color[i]);
	 //			layers[i]->set_roughness(roughness[i]);
	 ////			layers[i]->set_anisotropy(anisotropy[i]);
	 //			layers[i]->set_metallic(metallic[i]);
	 //			layers[i]->set_layer_thickness(thickness[i]);
	 //			if (layers[i]->get_index_of_refraction() != index_of_refraction[i])
	 //				layers[i]->set_index_of_refraction(index_of_refraction[i]);
	 //			if (layers[i]->get_attenuation_coefficient().x != absorption[i])
	 //				layers[i]->set_attenuation_coefficient(glm::vec3{ absorption[i] });
	 //			if (layers[i]->get_scattering_phase_parameter() != phase[i])
	 //				layers[i]->set_scattering_phase_parameter(phase[i]);
	 //
	 //			if (i != 0) {
	 //				bool enabled = layers[i - 1]->get_next_layer() != nullptr;
	 //				if (layer_enabled[i] != enabled)
	 //					layers[i - 1]->set_next_layer(layer_enabled[i] ? layers[i].get() : nullptr);
	 //			}
	 //		}
	 //
	 //		if (ImGui::Begin("Atmosphere", nullptr)) {
	 //			ImGui::SliderFloat((std::string("Sun zenith angle ##value")).data(), &sun_zenith, .0f, 2 * glm::pi<float>());
	 //			ImGui::SliderFloat((std::string("Mie scattering coefficient (10^-8) ##value##mie1")).data(), &mie_scattering_coefficient, .0f, 100.f, "%.5f", 3.f);
	 //			ImGui::SliderFloat((std::string("Mie absorption coefficient (10^-8) ##value##mie2")).data(), &mie_absorption_coefficient, .0f, 100.f, "%.5f", 3.f);
	 //			ImGui::SliderFloat((std::string("Debug dummy variable")).data(), &dummy, .0f, 1.f);
	 //		}
	 //
	 //		ImGui::End();
	 //
	 //		atmosphere.mie_absorption_coefficient = static_cast<double>(mie_absorption_coefficient) * 1e-8;
	 //		atmosphere.mie_scattering_coefficient = static_cast<double>(mie_scattering_coefficient) * 1e-8;
	 //		renderer.get().update_atmospherics_properties(atmosphere);
	 //
	 //		renderer.get().get_composer_program().set_uniform("dummy", dummy);
	 //	});
	 //
	 //
	 //	auto footer_text = text_manager.get().create_renderer();
	 //	auto footer_text_task = graphics::make_gpu_task("footer_text", footer_text.get(), nullptr);
	 //
	 //#ifndef STATIC_SCENE
	 //	renderer.get().add_gui_task(footer_text_task);
	 //#endif
	 //	renderer.get().add_gui_task(graphics::make_gpu_task("debug_gui", debug_gui_dispatchable.get(), nullptr));
	 //
	 //	glm::ivec2 last_pointer_pos;
	 //	float time = 0;
	 //	while (running) {
	 //		if (ctx.window_active()) {
	 //			auto time_delta = ctx.time_per_frame().count();
	 //
	 //			using namespace hid;
	 //			constexpr float movement_factor = 155.f;
	 //			if (ctx.get_key_status(keyboard::K::KeyW) == Status::KeyDown)
	 //				camera.step_forward(time_delta*movement_factor);
	 //			if (ctx.get_key_status(keyboard::K::KeyS) == Status::KeyDown)
	 //				camera.step_backward(time_delta*movement_factor);
	 //			if (ctx.get_key_status(keyboard::K::KeyA) == Status::KeyDown)
	 //				camera.step_left(time_delta*movement_factor);
	 //			if (ctx.get_key_status(keyboard::K::KeyD) == Status::KeyDown)
	 //				camera.step_right(time_delta*movement_factor);
	 //
	 //			constexpr float rotation_factor = .09f;
	 //			bool rotate_camera = mouse_down;
	 //
	 //			auto pp = ctx.get_pointer_position();
	 //			if (mouse_down && !debug_gui_dispatchable->is_gui_active()) {
	 //				auto diff_v = static_cast<glm::vec2>(last_pointer_pos - pp) * time_delta * rotation_factor;
	 //				camera.pitch_and_yaw(-diff_v.y, diff_v.x);
	 //			}
	 //			last_pointer_pos = pp;
	 //		}
	 //
	 //#ifdef STATIC_SCENE
	 //		glm::vec3 lp = light0_pos;
	 //		glm::vec3 sun_dir = sun_direction;
	 //#else
	 //		float angle = time * glm::pi<float>() / 2.5f;
	 //		glm::vec3 lp = light0_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 115.f;
	 //
	 //		glm::vec3 sun_dir = glm::normalize(glm::vec3{ glm::sin(sun_zenith + glm::pi<float>()), 
	 //													  -glm::cos(sun_zenith + glm::pi<float>()), 
	 //													  .15f});
	 //
	 //		light0.first->set_position(lp);
	 //		light0.second->set_model_transform(glm::mat4x3(glm::translate(glm::mat4(1.f), lp)));
	 //		sun_light->set_direction(sun_dir);
	 //#endif
	 //
	 //		{
	 //			using namespace text::attributes;
	 //
	 //			static unsigned tpf_count = 0;
	 //			static float total_tpf = .0f;
	 //			total_tpf += ctx.time_per_frame().count();
	 //			++tpf_count;
	 //			static float tpf = .0f;
	 //			if (tpf_count % 5 == 0) {
	 //				tpf = total_tpf / 5.f;
	 //				total_tpf = .0f;
	 //			}
	 //
	 //			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
	 //			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);
	 //
	 //			footer_text->set_text({ 10, 50 }, line_height(28)(vsmall(b(stroke(dark_magenta, 1)(red(std::to_wstring(tpf * 1000.f))))) + L" ms\n" +
	 //															  vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")))));
	 //		}
	 //
	 //		time += ctx.time_per_frame().count();
	 //		if (!ctx.tick()) break;
	 //	}
	 //
	 //	return 0;
	 //}
