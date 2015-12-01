
#include "stdafx.h"
#include "windows.h"

#include <glm/glm.hpp>

#include "gl_utils.h"
#include "Log.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "StEngineControl.h"
#include "GIRenderer.h"
#include "SphericalLight.h"
#include "DirectionalLight.h"
#include "BasicRenderer.h"
#include "hdr_dof_postprocess.h"
#include "MeshRenderable.h"
#include "CustomRenderable.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"
#include "SurfaceIO.h"
#include "Texture2D.h"
#include "Scene.h"
#include "TextManager.h"
#include "AttributedString.h"
#include "RGB.h"
#include "Sphere.h"

#include "dense_voxel_space.h"
#include "renderable.h"

using namespace StE::LLR;
using namespace StE::Text;

class SkyDome : public StE::Graphics::MeshRenderable<StE::Graphics::mesh_subdivion_mode::Triangles> {
private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;

	std::unique_ptr<Texture2D> stars_tex;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	SkyDome(const StE::StEngineControl &ctx) : MeshRenderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "transform_sky.vert", "frag_sky.frag" })(),
															  std::make_shared<StE::Graphics::Sphere>(10, 10, .0f)) {
		stars_tex = StE::Resource::SurfaceIO::load_texture_2d_task("Data/textures/stars.jpg", true)();

		get_program()->set_uniform("sky_luminance", 5.f);
		get_program()->set_uniform("projection", ctx.projection_matrix());
		get_program()->set_uniform("near", ctx.get_near_clip());
		get_program()->set_uniform("far", ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near, float clip_far) {
			this->get_program()->set_uniform("projection", proj);
			this->get_program()->set_uniform("near", clip_near);
			this->get_program()->set_uniform("far", clip_far);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("view_model", m);
	}

	virtual void prepare() const override {
		MeshRenderable::prepare();

		0_tex_unit = *stars_tex;
	}
};

class RayTracer : public StE::Graphics::renderable {
private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	RayTracer(const StE::StEngineControl &ctx) : StE::Graphics::renderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert", "ray.frag" })()) {
		get_program()->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near, float clip_far) {
			get_program()->set_uniform("inv_projection", glm::inverse(proj));
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("inv_view_model", glm::inverse(m));
	}

	void set_world_center(const glm::vec3 &c, float voxel_size) const {
		auto vs = voxel_size * 4;
		glm::vec3 translation = glm::round(c / vs) * vs;
		get_program()->set_uniform("translation", c - translation);
	}

	virtual void prepare() const override {
		renderable::prepare();

		StE::Graphics::ScreenFillingQuad.vao()->bind();
	}

	virtual void render() const override {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

glm::vec2 bounding_triangle_vertex(glm::vec3 U, glm::vec3 V, glm::vec3 W) {
	using namespace glm;

	vec4 prev = vec4(U, 1);
	vec4 v = vec4(V, 1);
	vec4 next = vec4(W, 1);

	vec3 a = v.xyw - prev.xyw;
	vec3 b = next.xyw - v.xyw;

	vec3 p0 = cross(a, vec3(prev.xyw));
	vec3 p1 = cross(b, vec3(v.xyw));

	p0.z -= dot(vec2(1), abs(vec2(p0.xy)));
	p1.z -= dot(vec2(1), abs(vec2(p1.xy)));

	vec3 t = cross(p0, p1);
	return vec2(t.xy) / t.z;
}

void do_stuff() {
	using namespace glm;

	vec3 U = { -100, -10, 0 };
	vec3 V =  { 100,-10,-1 };
	vec3 W = { -50,100,10 };

	vec3 T = V - U;
	vec3 N = normalize(cross(T, W - U));

	float d = dot(-U, N);
	float voxel = 4;// voxel_size(0);//voxel_level(d));

	vec3 pos0 = U / voxel;
	vec3 pos1 = V / voxel;
	vec3 pos2 = W / voxel;

	T = normalize(T);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);
	mat3 invTBN = transpose(TBN);

	float voxels_texture_size = 1.f * 1024.f;
	vec3 p0 = invTBN * pos0;
	vec3 p1 = invTBN * pos1;
	vec3 p2 = invTBN * pos2;
	vec2 minv = min(min(vec2(p0.xy), vec2(p1.xy)), vec2(p2.xy));

	p0.xy = bounding_triangle_vertex(p2, p0, p1);
	p1.xy = bounding_triangle_vertex(p0, p1, p2);
	p2.xy = bounding_triangle_vertex(p1, p2, p0);

	U = TBN * (p0 * voxel);
	V = TBN * (p1 * voxel);
	W = TBN * (p2 * voxel);

	vec2 v0 = p0.xy - minv;
	vec2 v1 = p1.xy - minv;
	vec2 v2 = p2.xy - minv;

	v0 /= voxels_texture_size;
	v1 /= voxels_texture_size;
	v2 /= voxels_texture_size;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdParam, int iCmdShow) {
	StE::Log logger("Simulation");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	do_stuff();

	int w = 1688, h = 950;
	constexpr float clip_far = 4096.f;
	constexpr float clip_near = 5.f;

	gl_context::context_settings settings;
	settings.vsync = false;
	StE::StEngineControl ctx(std::make_unique<gl_context>(settings, "Shlomi Steinberg - Simulation", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near, clip_far);

	using ResizeSignalConnectionType = StE::StEngineControl::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	StE::Graphics::SceneProperties scene_properties;
	StE::Graphics::GIRenderer renderer(ctx, &scene_properties);
	StE::Graphics::BasicRenderer basic_renderer;

	StE::LLR::Camera camera;
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ 26.4, 548.5, 248.71 });

	const glm::vec3 light_pos({ -700.6, 138, -70 });
	auto light0 = std::make_shared<StE::Graphics::SphericalLight>(2000.f, StE::Graphics::RGB({ 1.f, .57f, .16f }), light_pos, 10.f);
	auto light1 = std::make_shared<StE::Graphics::DirectionalLight>(1.f, StE::Graphics::RGB({ 1.f, 1.f, 1.f }), glm::normalize(glm::vec3(0.1f, -2.5f, 0.1f)));
	scene_properties.lights_storage().add_light(light0);
	scene_properties.lights_storage().add_light(light1);

	SkyDome skydome(ctx);
	StE::Graphics::Scene scene(ctx, &scene_properties);
	StE::Text::TextManager text_renderer(ctx, StE::Text::Font("Data/ArchitectsDaughter.ttf"));

	StE::Graphics::hdr_dof_postprocess hdr{ ctx, renderer.z_buffer() };

	renderer.set_output_fbo(hdr.get_input_fbo());

	StE::Graphics::CustomRenderable fb_clearer{ [&]() { ctx.gl()->clear_framebuffer(); } };
	StE::Graphics::CustomRenderable fb_depth_clearer{ [&]() { ctx.gl()->clear_framebuffer(false); } };


	bool running = true;

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


	bool loaded = false;
	auto model_future = ctx.scheduler().schedule_now(StE::Resource::ModelLoader::load_model_task(ctx, R"(data\models\crytek-sponza\sponza.obj)", &scene, 2.5f));

	std::shared_ptr<StE::Graphics::Object> light_obj;
	{
		std::unique_ptr<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>> m = std::make_unique<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>>();
		std::vector<StE::Graphics::ObjectVertexData> vertices;
		StE::Graphics::ObjectVertexData v;
		v.p = { -100,-10,0 };
		v.uv = { 0,0 };
		vertices.push_back(v);
		v.p = { 100,-10,0 };
		v.uv = { 1,0 };
		vertices.push_back(v);
		v.p = { -50,100,0 };
		v.uv = { 0,1 };
		vertices.push_back(v);
		m->set_vertices(vertices);
		m->set_indices(std::vector<unsigned>{0,1,2});
		light_obj = std::make_shared<StE::Graphics::Object>(std::move(m));

		std::unique_ptr<StE::Graphics::Sphere> sphere = std::make_unique<StE::Graphics::Sphere>(10, 10);
		light_obj = std::make_shared<StE::Graphics::Object>(std::move(sphere));

		light_obj->set_model_transform(glm::scale(glm::translate(glm::mat4(), light_pos), glm::vec3(10, 10, 10)));

		gli::texture2D light_color_tex{ 1, gli::format::FORMAT_RGB8_UNORM, {1,1} };
		glm::vec3 c = light0->get_diffuse();
		*reinterpret_cast<glm::u8vec3*>(light_color_tex.data()) = glm::u8vec3(c.r * 255.5f, c.g * 255.5f, c.b * 255.5f);

		auto light_mat = std::make_shared<StE::Graphics::Material>();
		light_mat->set_diffuse(std::make_shared<StE::LLR::Texture2D>(light_color_tex, false));
		light_mat->set_emission(c * light0->get_luminance());

		light_obj->set_material_id(scene_properties.material_storage().add_material(light_mat));

		scene.add_object(light_obj);
	}

	StE::Graphics::dense_voxel_space voxel_space(ctx, 1024, 1.f);
	RayTracer ray_tracer(ctx);
	voxel_space.update_shader_voxel_uniforms(*ray_tracer.get_program());


	ctx.set_renderer(&basic_renderer);

	while (!loaded && running) {
		ctx.renderer()->queue().push_back(&fb_clearer);

		{
			using namespace StE::Text::Attributes;
			AttributedWString str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
										   azure(large(L"Loading...\n")) +
										   orange(regular(L"By Shlomi Steinberg")));
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			ctx.renderer()->queue().push_back(text_renderer.render({ w / 2, h / 2 + 100 }, str));
			ctx.renderer()->queue().push_back(text_renderer.render({ 10, 20 }, vsmall(b(L"Thread pool workers: ") +
																					  olive(std::to_wstring(ctx.scheduler().get_sleeping_workers())) + 
																			 		  L"/" + 
																					  olive(std::to_wstring(ctx.scheduler().get_workers_count())))));
			ctx.renderer()->queue().push_back(text_renderer.render({ 10, 50 },
																   vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB"))));
		}

		if (model_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			loaded = true;

		ctx.run_loop();
	}


	//ctx.set_renderer(&renderer);

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
			auto pp = ctx.get_pointer_position();
			auto center = static_cast<glm::vec2>(ctx.get_backbuffer_size())*.5f;
			ctx.set_pointer_position(static_cast<glm::ivec2>(center));
			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta * rotation_factor;
			camera.pitch_and_yaw(-diff_v.y, diff_v.x); 
		}


		auto mv = camera.view_matrix();
		auto mvnt = camera.view_matrix_no_translation();

		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 135.f;

		light0->set_position(lp);

		light_obj->set_model_transform(glm::scale(glm::translate(glm::mat4(), lp), glm::vec3(light0->get_radius() / 2.f)));
		renderer.set_model_matrix(mv);
		scene.set_model_matrix(mv);
		skydome.set_model_matrix(mvnt);
		ray_tracer.set_model_matrix(mvnt);
		voxel_space.set_model_matrix(mv, camera.get_position());
		ray_tracer.set_world_center(camera.get_position(), voxel_space.get_voxel_texel_size());

//		renderer.queue().push_back(&fb_depth_clearer);
//		renderer.queue().push_back(&scene);
//		renderer.queue().push_back(&skydome);
//		renderer.postprocess_queue().push_back(&hdr);

		ctx.renderer()->queue().push_back(&fb_clearer);
		ctx.renderer()->queue().push_back(voxel_space.voxelizer(scene));
		ctx.renderer()->queue().push_back(&ray_tracer);

		{
			using namespace StE::Text::Attributes;
			auto tpf = std::to_wstring(ctx.time_per_frame().count());
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			ctx.renderer()->queue().push_back(text_renderer.render({ 30, h - 50 },
																		vsmall(b(stroke(dark_magenta, 1)(red(tpf)))) + L" ms"));
			/*renderer.postprocess_queue()*/ctx.renderer()->queue().push_back(text_renderer.render({ 30, 20 },
																		vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")))));
		}

		time += ctx.time_per_frame().count();
		if (!ctx.run_loop()) break;
	}
	 
	return 0;
}
