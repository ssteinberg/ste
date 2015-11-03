
#include "stdafx.h"
#include "windows.h"

#include <glm/glm.hpp>

#include <vector>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>
#include <numeric>

#include "gl_utils.h"
#include "Texture1D.h"
#include "Texture2D.h"
#include "Log.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "StEngineControl.h"
#include "gl_current_context.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "GLSLProgramLoader.h"
#include "SurfaceIO.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "FramebufferObject.h"
#include "RenderTarget.h"
#include "Texture2DArray.h"
#include "PixelBufferObject.h"
#include "AtomicCounterBufferObject.h"
#include "Scene.h"
#include "TextRenderer.h"
#include "AttributedString.h"
#include "human_vision_properties.h"
#include "RGB.h"
#include "bme_brdf_representation.h"
#include "Sphere.h"

using namespace StE::LLR;
using namespace StE::Text;

struct Vertex {
	glm::vec3 p;
	glm::vec2 t;
};

StE::LLR::Camera camera;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdParam, int iCmdShow) {
	StE::Log logger("Simulation");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	//	int w = 2560, h = 1440;
	int w = 1688, h = 950;
	constexpr float clip_far = 10000.f;
	constexpr float clip_near = 5.f;

	gl_context::context_settings settings;
	settings.vsync = false;
	//	settings.fs = true;
	StE::StEngineControl ctx(std::make_unique<gl_context>(settings, "Shlomi Steinberg - Simulation", glm::i32vec2{ w, h }));

	std::string gl_err_desc;
	//while (StE::LLR::opengl::query_gl_error(gl_err_desc));
// 	StE::Graphics::texture_pool tp;
// 	StE::LLR::Texture2D tt(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(256, 256), 5);
// 	StE::LLR::opengl::query_gl_error(gl_err_desc);
// 	auto itt = tp.commit(tt);
// 	tp.commit(tt);
// 	tp.uncommit(itt);
	//while (StE::LLR::opengl::query_gl_error(gl_err_desc));

	ctx.set_clipping_planes(clip_near, clip_far);
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ 26.4, 548.5, 248.71 });

	// Prepare
	StE::Graphics::Scene scene;
	StE::Text::TextRenderer text_renderer(ctx, StE::Text::Font("Data/ArchitectsDaughter.ttf"));

	std::unique_ptr<GLSLProgram> hdr_compute_minmax = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_compute_minmax.glsl" })();
	std::unique_ptr<GLSLProgram> transform = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "transform.vert", "frag.frag" })();
	std::unique_ptr<GLSLProgram> transform2 = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "transform2.vert", "frag2.frag" })();
	std::unique_ptr<GLSLProgram> transform_sky = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "transform_sky.vert", "frag_sky.frag" })();
	std::unique_ptr<GLSLProgram> deffered = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert", "lighting.frag" })();
	std::unique_ptr<GLSLProgram> hdr_create_histogram = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_create_histogram.glsl" })();
	std::unique_ptr<GLSLProgram> hdr_compute_histogram_sums = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "hdr_compute_histogram_sums.glsl" })();
	std::unique_ptr<GLSLProgram> hdr_tonemap_coc = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_tonemap_coc.frag" })();
	std::unique_ptr<GLSLProgram> hdr_bloom_blurx = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_x.frag" })();
	std::unique_ptr<GLSLProgram> hdr_bloom_blury = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","hdr_bloom_blur_y.frag" })();
	std::unique_ptr<GLSLProgram> bokeh_draw_bokeh_effects = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "bokeh_draw_bokeh_effects.vert","bokeh_draw_bokeh_effects.geom","bokeh_draw_bokeh_effects.frag" })();
	std::unique_ptr<GLSLProgram> bokeh_combine = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","bokeh_combine.frag" })();
	std::unique_ptr<GLSLProgram> bokeh_blurx = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_x.frag" })();
	std::unique_ptr<GLSLProgram> bokeh_blury = StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "passthrough.vert","bokeh_bilateral_blur_y.frag" })();

	StE::LLR::RenderTarget depth_output(gli::format::FORMAT_D24_UNORM, { w, h });
	StE::LLR::Texture2D normal_output(gli::format::FORMAT_RGB32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D tangent_output(gli::format::FORMAT_RGB32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D position_output(gli::format::FORMAT_RGB32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D color_output(gli::format::FORMAT_RGBA32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D material_idx_output(gli::format::FORMAT_R16_SINT, { w, h }, 1);
	StE::LLR::Texture2D z_output(gli::format::FORMAT_R32_SFLOAT, { w, h }, 1);
	StE::LLR::FramebufferObject fbo;
	fbo.depth_binding_point() = depth_output;
	fbo[0] = position_output[0];
	fbo[1] = color_output[0];
	fbo[2] = normal_output[0];
	fbo[3] = z_output[0];
	fbo[4] = tangent_output[0];
	fbo[5] = material_idx_output[0];
	1_color_idx = fbo[0];
	0_color_idx = fbo[1];
	2_color_idx = fbo[2];
	3_color_idx = fbo[3];
	4_color_idx = fbo[4];
	5_color_idx = fbo[5];

	using vertex_descriptor = StE::LLR::VBODescriptorWithTypes<glm::vec3, glm::vec2>::descriptor;
	using vbo_type = StE::LLR::VertexBufferObject<Vertex, vertex_descriptor>;
	std::shared_ptr<vbo_type> vbo = std::make_shared<vbo_type>(std::vector<Vertex>(
	{ { { -1.f, -1.f, .0f }, { .0f, .0f } },
	{ { 1.f, -1.f, .0f }, { 1.f, .0f } },
	{ { -1.f, 1.f, .0f }, { .0f, 1.f } },
	{ { 1.f, 1.f, .0f }, { 1.f, 1.f } } }
	));
	StE::LLR::VertexArrayObject vao;
	vao[0] = (*vbo)[1];
	vao[1] = (*vbo)[0];


	StE::Graphics::Sphere sphere(10, 10);
	StE::LLR::VertexBufferObject<StE::Graphics::ObjectVertexData, StE::Graphics::ObjectVertexData::descriptor> sphere_vbo{ sphere.get_vertices() };
	StE::LLR::ElementBufferObject<> sphere_ebo{ sphere.get_indices() };
	StE::LLR::VertexArrayObject sphere_vao;
	sphere_vao[0] = sphere_vbo[0];
	sphere_vao[1] = sphere_vbo[1];
	sphere_vao[2] = sphere_vbo[2];
	sphere_vao[3] = sphere_vbo[3];

	auto stars_tex = StE::Resource::SurfaceIO::load_texture_2d_task("Data/textures/stars.jpg", true)();
	StE::Graphics::Sphere sky_dome(10, 10, .0f);
	StE::LLR::VertexBufferObject<StE::Graphics::ObjectVertexData, StE::Graphics::ObjectVertexData::descriptor> sky_dome_vbo{ sky_dome.get_vertices() };
	StE::LLR::ElementBufferObject<> sky_dome_ebo{ sky_dome.get_indices() };
	StE::LLR::VertexArrayObject sky_dome_vao;
	sky_dome_vao[0] = sky_dome_vbo[0];
	sky_dome_vao[1] = sky_dome_vbo[1];
	sky_dome_vao[2] = sky_dome_vbo[2];
	sky_dome_vao[3] = sky_dome_vbo[3];


	StE::LLR::Texture2D bokeh_coc(gli::format::FORMAT_RG32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);

	StE::LLR::Texture2D hdr_image(gli::format::FORMAT_RGBA32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::Texture2D hdr_final_image(gli::format::FORMAT_RGBA32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::Texture2D hdr_bloom_image(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::FramebufferObject fbo_hdr_final;
	fbo_hdr_final[0] = hdr_final_image[0];
	StE::LLR::FramebufferObject fbo_hdr;
	fbo_hdr[0] = hdr_image[0];
	fbo_hdr[1] = hdr_bloom_image[0];
	fbo_hdr[2] = bokeh_coc[0];

	StE::LLR::Texture2D hdr_bloom_blurx_image(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::FramebufferObject fbo_hdr_bloom_blurx_image;
	fbo_hdr_bloom_blurx_image[0] = hdr_bloom_blurx_image[0];

	gli::texture1D hdr_human_vision_properties_data(1, gli::format::FORMAT_RGBA32_SFLOAT, glm::tvec1<std::size_t>{ 4096 });
	{
		glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
		for (unsigned i = 0; i < hdr_human_vision_properties_data.dimensions().x; ++i, ++d) {
			float f = glm::mix(StE::Graphics::human_vision_properties::min_luminance, 10.f, static_cast<float>(i) / static_cast<float>(hdr_human_vision_properties_data.dimensions().x));
			*d = { StE::Graphics::human_vision_properties::scotopic_vision(f),
				StE::Graphics::human_vision_properties::red_response(f),
				StE::Graphics::human_vision_properties::monochromaticity(f),
				StE::Graphics::human_vision_properties::visual_acuity(f) };
		}
	}
	Sampler hdr_vision_properties_sampler(TextureFiltering::Linear, TextureFiltering::Linear, 16);
	hdr_vision_properties_sampler.set_wrap_s(TextureWrapMode::ClampToEdge);
	StE::LLR::Texture1D hdr_vision_properties_texture(hdr_human_vision_properties_data, false);
	auto hdr_vision_properties_texture_handle = hdr_vision_properties_texture.get_texture_handle(hdr_vision_properties_sampler);

	Sampler linear_sampler;
	linear_sampler.set_min_filter(TextureFiltering::Linear);
	linear_sampler.set_mag_filter(TextureFiltering::Linear);
	SamplerMipmapped linear_mipmaps_sampler;
	linear_mipmaps_sampler.set_min_filter(TextureFiltering::Linear);
	linear_mipmaps_sampler.set_mag_filter(TextureFiltering::Linear);
	linear_mipmaps_sampler.set_mipmap_filter(TextureFiltering::Linear);

	bool first_frame = true;
	int luminance_w = w / 4, luminance_h = h / 4;
	struct hdr_bokeh_parameters {
		std::int32_t lum_min, lum_max;
		float focus;
	};
	float big_float = 10000.f;
	StE::LLR::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer(1);
	StE::LLR::ShaderStorageBuffer<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev(1);
 	StE::LLR::Texture2D hdr_lums(gli::format::FORMAT_R32_SFLOAT, StE::LLR::Texture2D::size_type(luminance_w, luminance_h), 1);
 	StE::LLR::PixelBufferObject<std::int32_t> hdr_bokeh_param_buffer_eraser(std::vector<std::int32_t>{ *reinterpret_cast<std::int32_t*>(&big_float), 0 });
	StE::LLR::AtomicCounterBufferObject<> histogram(128);
 	StE::LLR::ShaderStorageBuffer<unsigned> histogram_sums(128);

	StE::LLR::Texture2D bokeh_blur_image_x(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::FramebufferObject fbo_bokeh_blur_image;
	fbo_bokeh_blur_image[0] = bokeh_blur_image_x[0];

	bool running = true;

	ctx.gl()->enable_state(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	// Bind input
	auto keyboard_listner = std::make_shared<decltype(ctx)::hid_keyboard_signal_type::connection_type>(
		[&](StE::HID::keyboard::K key, int scanline, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;
		auto time_delta = ctx.time_per_frame().count();

		if (status != Status::KeyDown)
			return;

		if (key == keyboard::K::KeyESCAPE)
			running = false;
		if (key == keyboard::K::KeyPRINT_SCREEN)
			ctx.capture_screenshot();
	});
	ctx.hid_signal_keyboard().connect(keyboard_listner);

	ctx.set_pointer_hidden(true);

	bool loaded = false;
	auto model_future = ctx.scheduler().schedule_now(StE::Resource::ModelLoader::load_model_task(ctx, R"(data\models\crytek-sponza\sponza.obj)", &scene));

	const glm::vec3 light_pos(-850.6, 138, -180);
	auto light_diffuse = StE::Graphics::RGB({ 1.f, .57f, .16f });
	auto light_intensity = 1700.f;

	transform->bind();
	transform->set_uniform("near", clip_near);
	transform->set_uniform("far", clip_far);

	transform2->bind();
	transform2->set_uniform("near", clip_near);
	transform2->set_uniform("far", clip_far);
	transform2->set_uniform("light_diffuse", light_diffuse);
	transform2->set_uniform("light_luminance", light_intensity);
	transform2->set_uniform("light_pos", light_pos);

	transform_sky->bind();
	transform_sky->set_uniform("far", clip_far);
	transform_sky->set_uniform("sky_luminance", 1.f);

	deffered->bind();
	deffered->set_uniform("light_diffuse", light_diffuse);
	deffered->set_uniform("light_luminance", light_intensity);

	// Run main loop
	while (!loaded && running) {
		ctx.run_loop();
		ctx.gl()->clear_framebuffer();

		{
			using namespace StE::Text::Attributes;
			AttributedWString str = center(stroke(blue_violet, 2)(purple(vlarge(b(L"Loading Simulation...")))) +
											L"\n" + 
											orange(regular(L"By Shlomi Steinberg")));
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

 			text_renderer.render({ w / 2, h / 2 - 20 }, str);
 			text_renderer.render({ 10, 20 }, vsmall(b(L"Thread pool workers: ") +
 									olive(std::to_wstring(ctx.scheduler().get_sleeping_workers())) + 
 									L"/" + 
 									olive(std::to_wstring(ctx.scheduler().get_workers_count()))));
			text_renderer.render({ 10, 50 },
								 vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")));
		}

		if (model_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
			continue;

		loaded = true;
	}

	while (running) {
		if (!ctx.run_loop()) break;

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

		ctx.gl()->enable_depth_test();

		auto proj_mat = ctx.projection_matrix();
		auto mv = camera.view_matrix();
		auto mvnt = camera.view_matrix_no_translation();
		auto inverse_mv = glm::inverse(mv);
		auto transpose_inverse_mv = glm::transpose(inverse_mv);

		{
			fbo.bind();
			ctx.gl()->clear_framebuffer(false);

			transform->bind();
			transform->set_uniform("view_model", mv);
			transform->set_uniform("trans_inverse_view_model", transpose_inverse_mv);
			transform->set_uniform("projection", proj_mat);
			scene.render();

			transform2->bind();
			transform2->set_uniform("view_model", mv);
			transform2->set_uniform("trans_inverse_view_model", transpose_inverse_mv);
			transform2->set_uniform("projection", proj_mat);
			sphere_vao.bind();
			sphere_ebo.bind();
			glDrawElements(GL_TRIANGLES, sphere_ebo.size(), GL_UNSIGNED_INT, nullptr);

			ctx.gl()->cull_face(GL_FRONT);
			transform_sky->bind();
			transform_sky->set_uniform("view_model", mvnt);
			transform_sky->set_uniform("projection", proj_mat);
			sky_dome_vao.bind();
			sky_dome_ebo.bind();
			0_tex_unit = *stars_tex;
			glDrawElements(GL_TRIANGLES, sky_dome_ebo.size(), GL_UNSIGNED_INT, nullptr);
			ctx.gl()->cull_face(GL_BACK);
		}

		ctx.gl()->disable_depth_test();

		vao.bind();

		{
			fbo_hdr_final.bind();
			//ctx.gl()->defaut_framebuffer().bind();
			deffered->bind();
			deffered->set_uniform("light_pos", (mv * glm::vec4(light_pos, 1)).xyz);
			0_tex_unit = normal_output;
			1_tex_unit = position_output;
			3_tex_unit = color_output;
			4_tex_unit = tangent_output;
			5_tex_unit = material_idx_output;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		
		
		0_sampler_idx = linear_sampler;
		1_sampler_idx = linear_mipmaps_sampler;
		unsigned zero = 0;
		histogram.clear(gli::FORMAT_R32_UINT, &zero);
		hdr_bokeh_param_buffer_prev << hdr_bokeh_param_buffer;
		hdr_bokeh_param_buffer << hdr_bokeh_param_buffer_eraser;


		0_tex_unit = hdr_final_image;

		ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

		hdr_compute_minmax->bind();
		hdr_compute_minmax->set_uniform("time", ctx.time_per_frame().count());
		0_image_idx = hdr_lums[0];
		2_storage_idx = hdr_bokeh_param_buffer;
		3_storage_idx = first_frame ? hdr_bokeh_param_buffer : hdr_bokeh_param_buffer_prev;
		glDispatchCompute(luminance_w / 32, luminance_h / 32, 1);

		ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		hdr_create_histogram->bind();
		0_atomic_idx = histogram;
		glDispatchCompute(luminance_w / 32, luminance_h / 32, 1);

		ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

		hdr_compute_histogram_sums->bind();
		hdr_compute_histogram_sums->set_uniform("time", ctx.time_per_frame().count());
		hdr_compute_histogram_sums->set_uniform("hdr_lum_resolution", static_cast<int>(luminance_w * luminance_h));
		0_storage_idx = histogram_sums;
		1_storage_idx = buffer_object_cast<ShaderStorageBuffer<unsigned>>(histogram);
		2_tex_unit = z_output;
		glDispatchCompute(1, 1, 1);

		ctx.gl()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

		fbo_hdr.bind();
		hdr_tonemap_coc->bind();
		3_sampler_idx = hdr_vision_properties_sampler;
		3_tex_unit = hdr_vision_properties_texture;
		hdr_tonemap_coc->set_uniform("aperature_radius", .0225f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		fbo_hdr_bloom_blurx_image.bind();
		hdr_bloom_blurx->bind();
		1_tex_unit = hdr_bloom_image;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		fbo_hdr_final.bind();
		hdr_bloom_blury->bind();
		0_tex_unit = hdr_image;
		1_tex_unit = hdr_bloom_blurx_image;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		0_tex_unit = hdr_final_image;
		1_tex_unit = bokeh_coc;
		fbo_bokeh_blur_image.bind();
		bokeh_blurx->bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		bokeh_blury->bind();
		ctx.gl()->defaut_framebuffer().bind();
		2_tex_unit = bokeh_blur_image_x;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		

		{
			using namespace StE::Text::Attributes;
			auto tpf = std::to_wstring(ctx.time_per_frame().count());
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			text_renderer.render({ 30, h - 50 },
								 vsmall(b(stroke(dark_magenta, 1)(red(tpf)))) + L" ms");
			text_renderer.render({ 30, 20 },
								 vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB"))));
		}

		first_frame = false;
	}
	 
	return 0;
}
