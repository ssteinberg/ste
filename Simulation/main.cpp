
#include "stdafx.h"
#include "windows.h"

#include <glm/glm.hpp>

#include <vector>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>

#include "opengl.h"
#include "Log.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "StEngineControl.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "ShaderLoader.h"
#include "SurfaceIO.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "FramebufferObject.h"
#include "RenderTarget.h"
#include "Texture2DArray.h"
#include "PixelBufferObject.h"
#include "AtomicCounterBuffer.h"
#include "Scene.h"

using namespace StE::LLR;

struct Vertex {
	glm::vec3 p;
	glm::vec2 t;
};

StE::LLR::Camera camera;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdParam, int iCmdShow) {
	StE::Log logger("Simulation");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation running";

	constexpr float w = 1400, h = 900;
	constexpr int max_steps = 8;
	constexpr int depth_layers_count = 3;
	constexpr float clip_far = 1000.f;
	constexpr float clip_near = 1.f;

	StE::StEngineControl rc;
	rc.init_render_context("Shlomi Steinberg - Simulation", { w, h }, false, false);

	std::string gl_err_desc;
	while (StE::LLR::opengl::query_gl_error(gl_err_desc));
// 	StE::Graphics::texture_pool tp;
// 	StE::LLR::Texture2D tt(gli::format::FORMAT_RGBA8_UNORM, StE::LLR::Texture2D::size_type(256, 256), 5);
// 	StE::LLR::opengl::query_gl_error(gl_err_desc);
// 	auto itt = tp.commit(tt);
// 	tp.commit(tt);
// 	tp.uncommit(itt);
 	//while (StE::LLR::opengl::query_gl_error(gl_err_desc));

	rc.set_clipping_planes(clip_near, clip_far);
	camera.set_position({ -91.0412979, 105.631607, -60.2330551 });
	camera.lookat({ -91.9486542, 105.291336, -59.98624 });

	// Prepare
	StE::Graphics::Scene scene;

	StE::LLR::GLSLProgram transform;
	transform.add_shader(StE::Resource::ShaderLoader::compile_from_path("transform.vert"));
	transform.add_shader(StE::Resource::ShaderLoader::compile_from_path("frag.frag"));
	transform.link();

	StE::LLR::GLSLProgram gen_depth_layers;
	gen_depth_layers.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	gen_depth_layers.add_shader(StE::Resource::ShaderLoader::compile_from_path("gen_depth_layers.frag"));
	gen_depth_layers.link();

	StE::LLR::GLSLProgram ssao;
	ssao.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	ssao.add_shader(StE::Resource::ShaderLoader::compile_from_path("ssao.frag"));
	ssao.link();

	StE::LLR::GLSLProgram blur_x;
	blur_x.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	blur_x.add_shader(StE::Resource::ShaderLoader::compile_from_path("blur_x.frag"));
	blur_x.link();

	StE::LLR::GLSLProgram blur_y;
	blur_y.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	blur_y.add_shader(StE::Resource::ShaderLoader::compile_from_path("blur_y.frag"));
	blur_y.link();

	StE::LLR::GLSLProgram deffered;
	deffered.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough_light.vert"));
	deffered.add_shader(StE::Resource::ShaderLoader::compile_from_path("lighting.frag"));
	deffered.link();

	StE::LLR::GLSLProgram hdr_create_luminance;
	hdr_create_luminance.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	hdr_create_luminance.add_shader(StE::Resource::ShaderLoader::compile_from_path("hdr_create_luminance.frag"));
	hdr_create_luminance.link();

	StE::LLR::GLSLProgram hdr_luminance_downsample;
	hdr_luminance_downsample.add_shader(StE::Resource::ShaderLoader::compile_from_path("hdr_lum_downsample.glsl"));
	hdr_luminance_downsample.link();

	constexpr int noise_size_w = 28;
	constexpr int noise_size_h = 28;
	gli::texture2D tex(1, gli::format::FORMAT_RG32_SFLOAT, { noise_size_w, noise_size_h });
	{
		std::random_device rd;
		std::uniform_real_distribution<float> ud(-1, 1);
		glm::vec2 *vectors = reinterpret_cast<glm::vec2*>(tex.data());
		for (int i = 0; i < noise_size_w * noise_size_h; ++i, ++vectors) {
			auto r1 = ud(rd);
			auto r2 = ud(rd);
			if (!r1 && !r2) r1 = 1;
			glm::vec2 v(static_cast<float>(r1), static_cast<float>(r2));
			v = glm::normalize(v);
			*vectors = v;
		}
	}
	StE::LLR::Texture2D noise(tex.format(), tex.dimensions(), 1, sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest));
	noise.upload(tex);

	StE::LLR::RenderTarget depth_output(gli::format::FORMAT_D24_UNORM, StE::LLR::Texture2D::size_type(w, h));
	StE::LLR::Texture2D normal_output(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1, 
									  sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest));
	StE::LLR::Texture2D position_output(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::Texture2D color_output(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1, 
									 sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest));
	StE::LLR::FramebufferObject fbo;
	fbo.depth_binding_point() = depth_output;
	fbo[0] = position_output[0];
	fbo[1] = color_output[0];
	fbo[2] = normal_output[0];
	1_color_idx = fbo[0];
	0_color_idx = fbo[1];
	2_color_idx = fbo[2];

	StE::LLR::Texture2D occlusion_final1_output(gli::format::FORMAT_R8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1, 
												sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge));
	StE::LLR::FramebufferObject fbo_final1;
	fbo_final1[0] = occlusion_final1_output[0];

	StE::LLR::Texture2D occlusion_final2_output(gli::format::FORMAT_R8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1,
												sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge));
	StE::LLR::FramebufferObject fbo_final2;
	fbo_final2[0] = occlusion_final2_output[0];

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

	StE::LLR::Texture2DArray depth_layers(gli::format::FORMAT_R32_UINT, StE::LLR::Texture2DArray::size_type(w, h, depth_layers_count), 1,
										  sampler_descriptor(TextureFiltering::Nearest, TextureFiltering::Nearest, TextureWrapMode::ClampToEdge, TextureWrapMode::ClampToEdge));
	StE::LLR::FramebufferObject fbo_depth_layers;
	for (int i = 0; i < depth_layers_count; ++i)
		fbo_depth_layers[i] = depth_layers[0][i].with_format(gli::format::FORMAT_R32_SINT);

	StE::LLR::Texture2DArray f_depth_layers(gli::format::FORMAT_R32_SFLOAT, StE::LLR::Texture2DArray::size_type(w, h, depth_layers_count), max_steps);
	StE::LLR::SamplerMipmapped depth_sampling_descriptor(StE::LLR::TextureWrapMode::ClampToEdge, StE::LLR::TextureWrapMode::ClampToEdge);
	StE::LLR::FramebufferObject fbo_f_depth_layers;
	for (int i = 0; i < depth_layers_count; ++i)
		fbo_f_depth_layers[i] = f_depth_layers[0][i];


	int luminance_w = 1, luminance_h = 1;
	while (luminance_w * 2 < w && luminance_h * 2 < h) {
		luminance_w *= 2;
		luminance_h *= 2;
	}
	int hdr_downsampling_levels = StE::LLR::Texture2D::calculate_mipmap_max_level({ luminance_w/2, luminance_h/2 }) + 1;
	std::vector<std::unique_ptr<StE::LLR::Texture2D>> luminance_downsampled(hdr_downsampling_levels);
	for (int i = 0; i < hdr_downsampling_levels; ++i) {
		auto size = StE::LLR::Texture2D::size_type(std::max(1, luminance_w >> (i+1)), std::max(1, luminance_h >> (i+1)));
		luminance_downsampled[i] = std::make_unique<StE::LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT, size, 1);
	}

	StE::LLR::Texture2D hdr_image(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::FramebufferObject fbo_hdr_image;
	fbo_hdr_image[0] = hdr_image[0];
	StE::LLR::Texture2D luminance_image(gli::format::FORMAT_RGBA32_SFLOAT, StE::LLR::Texture2D::size_type(luminance_w, luminance_h), 1);
	StE::LLR::FramebufferObject fbo_luminance_image;
	fbo_luminance_image[0] = luminance_image[0];

	StE::LLR::AtomicCounterBuffer<StE::LLR::BufferUsage::BufferUsageMapRead> histogram(64);

	int steps = max_steps;
	bool perform_ssao = true;
	bool running = true;

	// Bind input
	auto keyboard_listner = std::make_shared<StE::connection<StE::HID::keyboard::K, int, StE::HID::Status, StE::HID::ModifierBits>>(
		[&](StE::HID::keyboard::K key, int scanline, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;
		auto time_delta = rc.time_per_frame().count();

		if (status != Status::KeyDown)
			return;

		if (key == keyboard::K::KeyKP_ADD)
			steps = max_steps;
		if (key == keyboard::K::KeyKP_SUBTRACT)
			steps = 2;
		if (key == keyboard::K::Key0)
			perform_ssao = false;
		if (key == keyboard::K::Key9)
			perform_ssao = true;
		if (key == keyboard::K::KeyESCAPE)
			running = false;
		if (key == keyboard::K::KeyPRINT_SCREEN) {
			auto size = rc.render_context().framebuffer_size();
			gli::texture2D tex(gli::FORMAT_RGB8_UNORM, size);

			StE::LLR::FramebufferObject fbo;
			StE::LLR::Texture2D fbo_tex(gli::format::FORMAT_RGB8_UNORM, size, 1);
			fbo[0] = fbo_tex[0];

			rc.render_context().defaut_framebuffer().blit_to(fbo,size,size);
			fbo[0].read_pixels(tex.data(), 3 * size.x * size.y);

			rc.scheduler().schedule_now(StE::Resource::SurfaceIO::write_surface_2d_task(tex, R"(D:\a.png)"));
		};
	});
	rc.hid_signal_keyboard().connect(keyboard_listner);

	rc.set_pointer_hidden(true);

	ste_log_query_and_log_gl_errors();

	bool loaded = false;
	auto model_future = rc.scheduler().schedule_now(StE::Resource::ModelLoader::load_model_task(R"(data\models\sponza.obj)", &scene));

	// Run main loop
	rc.run_loop([&]() {
		if (!loaded) {
			if (model_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				return true;
			loaded = true;
		}

		if (rc.window_active()) {
			auto time_delta = rc.time_per_frame().count();

			using namespace StE::HID;
			constexpr float movement_factor = 35.f;
			if (rc.get_key_status(keyboard::K::KeyW) == Status::KeyDown)
				camera.step_forward(time_delta*movement_factor);
			if (rc.get_key_status(keyboard::K::KeyS) == Status::KeyDown)
				camera.step_backward(time_delta*movement_factor);
			if (rc.get_key_status(keyboard::K::KeyA) == Status::KeyDown)
				camera.step_left(time_delta*movement_factor);
			if (rc.get_key_status(keyboard::K::KeyD) == Status::KeyDown)
				camera.step_right(time_delta*movement_factor);

			constexpr float rotation_factor = .05f;
			auto pp = rc.get_pointer_position();
			auto center = static_cast<glm::vec2>(rc.get_backbuffer_size())*.5f;
			rc.set_pointer_position(static_cast<glm::ivec2>(center));
			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta * rotation_factor;
			camera.pitch_and_yaw(-diff_v.y, diff_v.x);
		}

		unsigned zero = 0;
		histogram.clear(gli::FORMAT_R32_UINT, &zero);

		for (int i = 0; i < depth_layers_count; ++i)
			depth_layers[0][i].with_format(gli::format::FORMAT_R32_SINT).bind(image_layout_binding(i));

		auto proj_mat = rc.projection_matrix();

		fbo_depth_layers.bind();
		rc.render_context().clear_framebuffer(true, false);

		rc.render_context().enable_depth_test();
		fbo.bind();
		rc.render_context().clear_framebuffer(false, true);

		transform.bind();
		auto mv = camera.view_matrix() * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-100, -35, 0)), glm::vec3(.25, .25, .25));
		transform.set_uniform("view_model", mv);
		transform.set_uniform("trans_inverse_view_model", glm::transpose(glm::inverse(mv)));
		transform.set_uniform("projection", proj_mat);
		scene.render();

		rc.render_context().disable_depth_test();

		vao.bind();
		vao.enable_vertex_attrib_array(0);
		vao.enable_vertex_attrib_array(1);

		if (perform_ssao) {
			fbo_f_depth_layers.bind();
			gen_depth_layers.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			f_depth_layers.generate_mipmaps();

			fbo_final1.bind();
			ssao.bind();
			ssao.set_uniform("steps", steps);
			ssao.set_uniform("proj_inv", glm::inverse(proj_mat));
			0_sampler_idx = normal_output;
			1_sampler_idx = position_output;
			2_sampler_idx = noise;
			3_sampler_idx = f_depth_layers;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final2.bind();
			blur_x.bind();
			0_sampler_idx = occlusion_final1_output;
			1_sampler_idx = position_output;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final1.bind();
			blur_y.bind();
			0_sampler_idx = occlusion_final2_output;
			1_sampler_idx = position_output;
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		fbo_hdr_image.bind();
		deffered.bind();
		deffered.set_uniform("ssao", perform_ssao);
		deffered.set_uniform("view", camera.view_matrix());
		0_sampler_idx = normal_output;
		1_sampler_idx = position_output;
		2_sampler_idx = occlusion_final1_output;
		3_sampler_idx = color_output;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glViewport(0, 0, luminance_w, luminance_h);
		fbo_luminance_image.bind();
		hdr_create_luminance.bind();
		0_sampler_idx = hdr_image;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		hdr_luminance_downsample.bind();
		for (int i = 0; i < hdr_downsampling_levels; ++i) {
			if (i == 0)
				0_image_idx = luminance_image[0].with_access(ImageAccessMode::Read);
			else
				0_image_idx = (*luminance_downsampled[i - 1])[0].with_access(ImageAccessMode::Read);
			1_image_idx = (*luminance_downsampled[i])[0].with_access(ImageAccessMode::Write);
			auto size = luminance_downsampled[i]->get_image_size();
			glm::uvec2 group_size = { std::min<unsigned>(128, size.x), std::min<unsigned>(128, size.y) };
			glDispatchComputeGroupSizeARB(size.x / group_size.x, size.y / group_size.y, 1,
										  group_size.x, group_size.y, 1);
		}

		glViewport(0, 0, w, h);

// 		0_atomic_idx = histogram;
// 
// 		auto map_ptr = histogram.map_read(64);
// 		auto histogram_data = map_ptr.get();
// 
 		//rc.render_context().defaut_framebuffer().bind();

		ste_log_query_and_log_gl_errors();

		return running;
	});
	 
	return 0;
}
