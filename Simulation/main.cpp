
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#include <Windows.h>

#include <glm/glm.hpp>

#include <vector>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>

#include "opengl.h"
#include "Log.h"
#include "concurrent_queue.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "RenderControl.h"
#include "Model.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "ShaderLoader.h"
#include "SurfaceIO.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "FramebufferObject.h"
#include "RenderTarget.h"
#include "Texture2DArray.h"

struct Vertex {
	glm::vec3 p;
	glm::vec2 t;
};

StE::LLR::Camera camera;

int CALLBACK WinMain(_In_  HINSTANCE hInstance,
					 _In_  HINSTANCE hPrevInstance,
					 _In_  LPSTR lpCmdLine,
					 _In_  int nCmdShow) {
	StE::Log logger("Simulation");
	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);

	ste_log() << "Simulation running";

	constexpr float w = 1400, h = 900;
	constexpr int max_steps = 8;
	constexpr int depth_layers_count = 3;
	constexpr float clip_far = 1000.f;
	constexpr float clip_near = 1.f;

	StE::LLR::RenderControl rc;
	rc.init_render_context("Shlomi Steinberg - Simulation", w, h);
	rc.set_clipping_planes(clip_near, clip_far);
	camera.set_position({ -91.0412979, 105.631607, -60.2330551 });
	camera.lookat({ -91.9486542, 105.291336, -59.98624 });

	StE::LLR::opengl::dump_gl_info(false);

	// Prepare
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

	constexpr int noise_size_w = 25;
	constexpr int noise_size_h = 25;
	gli::texture2D tex(1, gli::format::FORMAT_RG32_SFLOAT, { noise_size_w, noise_size_h });
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
	StE::LLR::Texture2D noise(tex);
	noise.set_min_filter(GL_NEAREST);
	noise.set_mag_filter(GL_NEAREST);

	StE::LLR::Texture2D depth_output(gli::format::FORMAT_D24_UNORM, { w, h }, 1);
	StE::LLR::Texture2D normal_output(gli::format::FORMAT_RGB32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D t = std::move(normal_output);
	normal_output.set_min_filter(GL_NEAREST);
	normal_output.set_mag_filter(GL_NEAREST);
	StE::LLR::Texture2D position_output(gli::format::FORMAT_RGB32_SFLOAT, { w, h }, 1);
	StE::LLR::Texture2D color_output(gli::format::FORMAT_RGB8_UNORM, { w, h }, 1);
	color_output.set_min_filter(GL_NEAREST);
	color_output.set_mag_filter(GL_NEAREST);
	StE::LLR::FramebufferObject fbo;
	fbo.set_attachments({ depth_output }, { color_output, position_output, normal_output });

	StE::LLR::Texture2D occlusion_final1_output(gli::format::FORMAT_R8_UNORM, { w, h }, 1);
	occlusion_final1_output.set_min_filter(GL_NEAREST);
	occlusion_final1_output.set_mag_filter(GL_NEAREST);
	occlusion_final1_output.set_wrap_s(GL_CLAMP_TO_EDGE);
	occlusion_final1_output.set_wrap_t(GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_final1;
	fbo_final1.set_attachments({ occlusion_final1_output });

	StE::LLR::Texture2D occlusion_final2_output(gli::format::FORMAT_R8_UNORM, { w, h }, 1);
	occlusion_final2_output.set_min_filter(GL_NEAREST);
	occlusion_final2_output.set_mag_filter(GL_NEAREST);
	occlusion_final2_output.set_wrap_s(GL_CLAMP_TO_EDGE);
	occlusion_final2_output.set_wrap_t(GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_final2;
	fbo_final2.set_attachments({ occlusion_final2_output });
	std::size_t ts = occlusion_final2_output.get_storage_size(0);

	using vertex_descriptor = StE::LLR::VBODescriptorWithTypes<glm::vec3, glm::vec2>::descriptor;
	std::shared_ptr<StE::LLR::VertexBufferObject<Vertex, vertex_descriptor>> vbo = std::make_shared<StE::LLR::VertexBufferObject<Vertex, vertex_descriptor>>(std::vector<Vertex>(
		{ { { -1.f, -1.f, .0f }, { .0f, .0f } },
		{ { 1.f, -1.f, .0f }, { 1.f, .0f } },
		{ { -1.f, 1.f, .0f }, { .0f, 1.f } },
		{ { 1.f, 1.f, .0f }, { 1.f, 1.f } } }
	));
	StE::LLR::VertexArrayObject vao;
	vao[0] = (*vbo)[1];
	vao[1] = (*vbo)[0];

	StE::LLR::Texture2DArray depth_layers(gli::format::FORMAT_R32_UINT, { w, h, depth_layers_count }, 1);
	depth_layers.set_mag_filter(GL_NEAREST);
	depth_layers.set_min_filter(GL_NEAREST);
	depth_layers.set_wrap_s(GL_CLAMP_TO_EDGE);
	depth_layers.set_wrap_t(GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_depth_layers;
	fbo_depth_layers.bind();
	for (int i = 0; i < depth_layers_count; ++i)
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, depth_layers.get_resource_id(), 0, i);

	StE::LLR::Texture2DArray f_depth_layers(gli::format::FORMAT_R32_SFLOAT, { w, h, depth_layers_count }, max_steps);
	f_depth_layers.set_mag_filter(GL_LINEAR);
	f_depth_layers.set_min_filter(GL_LINEAR_MIPMAP_NEAREST);
	f_depth_layers.set_wrap_s(GL_CLAMP_TO_EDGE);
	f_depth_layers.set_wrap_t(GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_f_depth_layers;
	fbo_f_depth_layers.bind();
	for (int i = 0; i < depth_layers_count; ++i)
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, f_depth_layers.get_resource_id(), 0, i);

	StE::Resource::Model hall_model, obj1_model;
	hall_model.load_model(R"(data\models\sponza.obj)");

	StE::LLR::opengl::query_gl_error(__FILE__, __LINE__);

	int steps = max_steps;
	bool perform_ssao = true;

	// Run main loop
	auto time = std::chrono::high_resolution_clock::now();
	float time_total = .0f;
	int frames = 0;
	rc.run_loop([&]() {
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_delta = now - time;
		time = now;

		time_total += time_delta.count();
		if (frames % 40 == 0) {
			std::cout << "Time per frame: " << time_total / 40.0f << "sec" << std::endl;
			time_total = 0;
		}
		++frames;

		if (rc.window_active())	{	
			// Handle keyboard input
			constexpr float movement_factor = 35.f;
			using StE::HID::KeyboardInput;
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::W))
				camera.step_forward(time_delta.count()*movement_factor);
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::S))
				camera.step_backward(time_delta.count()*movement_factor);
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::A))
				camera.step_left(time_delta.count()*movement_factor);
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::D))
				camera.step_right(time_delta.count()*movement_factor);
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::Add))
				steps = max_steps;
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::Subtract))
				steps = 2;
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::Num0))
				perform_ssao = false;
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::Num9))
				perform_ssao = true;
			if (KeyboardInput::is_key_pressed(KeyboardInput::K::Escape))
				return false;

// 			// Handle mouse input
			constexpr float rotation_factor = .05f;
			using StE::HID::PointerInput;
			auto pp = PointerInput::position();
			auto center = static_cast<glm::vec2>(rc.viewport_size())*.5f;
			PointerInput::set_position(static_cast<glm::ivec2>(center));
			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta.count() * rotation_factor;
			camera.pitch_and_yaw(diff_v.y, diff_v.x);
		}

		auto proj_mat = rc.projection_matrix();

		fbo_depth_layers.bind();
		fbo_depth_layers.set_rendering_targets({ { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } });
		rc.render_context().clear_framebuffer(true, false);

		rc.render_context().enable_depth_test();
		fbo.bind();
		fbo.set_rendering_targets({ { 0, 0 }, { 1, 1 }, { 2, 2 } });
		rc.render_context().clear_framebuffer(false, true);

		for (int i = 0; i < depth_layers_count; ++i)
			glBindImageTexture(i, depth_layers.get_resource_id(), 0, false, i, GL_READ_WRITE, GL_R32I);
		transform.bind();
		auto mv = camera.view_matrix() * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-100, -35, 0)), glm::vec3(.25, .25, .25));
		transform.set_uniform("view_model", mv);
		transform.set_uniform("trans_inverse_view_model", glm::transpose(glm::inverse(mv)));
		transform.set_uniform("projection", proj_mat);
		hall_model.render(transform);

		rc.render_context().disable_depth_test();

		vao.bind();
		vao.enable_vertex_attrib_array(0);
		vao.enable_vertex_attrib_array(1);

		if (perform_ssao) {
			fbo_f_depth_layers.bind();
			fbo_f_depth_layers.set_rendering_targets({ { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } });
			gen_depth_layers.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			f_depth_layers.generate_mipmaps();

			f_depth_layers.bind(3);
			fbo_final1.bind();
			ssao.bind();
			ssao.set_uniform("steps", steps);
			ssao.set_uniform("proj_inv", glm::inverse(proj_mat));
			normal_output.bind(0);
			position_output.bind(1);
			noise.bind(2);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final2.bind();
			blur_x.bind();
			occlusion_final1_output.bind(0);
			position_output.bind(1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final1.bind();
			blur_y.bind();
			occlusion_final2_output.bind(0);
			position_output.bind(1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		fbo_final1.unbind();
		deffered.bind();
		deffered.set_uniform("ssao", perform_ssao);
		deffered.set_uniform("view", camera.view_matrix());
		normal_output.bind(0);
		position_output.bind(1);
		occlusion_final1_output.bind(2);
		color_output.bind(3);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		StE::LLR::opengl::query_gl_error(__FILE__, __LINE__);

		return true;
	});
	 
	return 0;
}
