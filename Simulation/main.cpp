
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <Windows.h>

#include <glm/glm.hpp>

#include <vector>
#include <random>
#include <iostream>
#include <chrono>

#include "glutils.h"
#include "Log.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "RenderControl.h"
#include "AssImpModel.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "ShaderLoader.h"
#include "SurfaceIO.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "FramebufferObject.h"

StE::LLR::Camera camera;

int CALLBACK WinMain(_In_  HINSTANCE hInstance,
					 _In_  HINSTANCE hPrevInstance,
					 _In_  LPSTR lpCmdLine,
					 _In_  int nCmdShow) {
	ste_log() << "Simulation running";

	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	constexpr float w = 1400, h = 900;
	constexpr int max_steps = 6;
	constexpr int depth_layers_count = 3;

	auto &rc = StE::LLR::RenderControl::instance();
	rc.init_render_context("Shlomi Steinberg - Simulation", w, h);
	rc.set_clipping_planes(1, 1000);
	camera.set_position({ -91.0412979, 105.631607, -60.2330551 });
	camera.lookat({ -91.9486542, 105.291336, -59.98624 });

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

	StE::LLR::GLSLProgram blur;
	blur.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough.vert"));
	blur.add_shader(StE::Resource::ShaderLoader::compile_from_path("blur.frag"));
	blur.link();

	StE::LLR::GLSLProgram deffered;
	deffered.add_shader(StE::Resource::ShaderLoader::compile_from_path("passthrough_light.vert"));
	deffered.add_shader(StE::Resource::ShaderLoader::compile_from_path("lighting.frag"));
	deffered.link();

	StE::Resource::AssImpModel hall_model, obj1_model;
	hall_model.load_model(R"(models\sponza.obj)");
	//obj1_model.load_model(R"(models\tess5.obj)");

	constexpr int noise_size_w = 25;
	constexpr int noise_size_h = 25;
	gli::texture2D tex(1, gli::format::FORMAT_RG32_SFLOAT, { noise_size_w, noise_size_h });
	std::random_device rd;
	std::uniform_real_distribution<float> ud(-1,1);
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

 	StE::LLR::Texture2D depth_output(gli::format::FORMAT_D24_UNORM, w, h, 1);
	StE::LLR::Texture2D normal_output(gli::format::FORMAT_RGB32_SFLOAT, w, h, 1);
	normal_output.set_min_filter(GL_NEAREST);
	normal_output.set_mag_filter(GL_NEAREST);
	StE::LLR::Texture2D position_output(gli::format::FORMAT_RGB32_SFLOAT, w, h, 1);
	StE::LLR::Texture2D color_output(gli::format::FORMAT_RGB8_UNORM, w, h, 1);
	color_output.set_min_filter(GL_NEAREST);
	color_output.set_mag_filter(GL_NEAREST);
 	StE::LLR::FramebufferObject fbo;
	fbo.set_attachments({ depth_output }, { color_output, position_output, normal_output });

	StE::LLR::Texture2D occlusion_final1_output(gli::format::FORMAT_R8_UNORM, w, h, 1);
	occlusion_final1_output.set_min_filter(GL_NEAREST);
	occlusion_final1_output.set_mag_filter(GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_final1;
	fbo_final1.set_attachments({ occlusion_final1_output });

	StE::LLR::Texture2D occlusion_final2_output(gli::format::FORMAT_R8_UNORM, w, h, 1);
	occlusion_final2_output.set_min_filter(GL_NEAREST);
	occlusion_final2_output.set_mag_filter(GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	StE::LLR::FramebufferObject fbo_final2;
	fbo_final2.set_attachments({ occlusion_final2_output });

	StE::LLR::VertexBufferObject vbo;
	StE::LLR::VertexArrayObject vao;
	{
		std::vector<float> vbo_data({ -1.f, -1.f, .0f, .0f, .0f,
									1.f, -1.f, .0f, 1.f, .0f,
									-1.f, 1.f, .0f, .0f, 1.f,
									1.f, 1.f, .0f, 1.f, 1.f });
		vbo.append(&vbo_data[0], sizeof(float) * vbo_data.size());
		vbo.upload(GL_ARRAY_BUFFER, GL_STATIC_DRAW);

		vao.bind();
		vbo.bind_vertex_buffer_to_array(0, 0, sizeof(float) * 5, 3, GL_FLOAT, false);
		vbo.bind_vertex_buffer_to_array(1, sizeof(float)*3, sizeof(float) * 5, 2, GL_FLOAT, false);
	}

	GLuint depth_layers, f_depth_layers;
	glGenTextures(1, &depth_layers);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depth_layers);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32I, w, h, depth_layers_count);
	StE::LLR::FramebufferObject fbo_depth_layers;
	fbo_depth_layers.bind();
	for (int i = 0; i < depth_layers_count; ++i)
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, depth_layers, 0, i);

	glGenTextures(1, &f_depth_layers);
	glBindTexture(GL_TEXTURE_2D_ARRAY, f_depth_layers);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, max_steps-1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, max_steps, GL_R32F, w, h, depth_layers_count);
	StE::LLR::FramebufferObject fbo_f_depth_layers;
	fbo_f_depth_layers.bind();
	for (int i = 0; i < depth_layers_count; ++i)
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, f_depth_layers, 0, i);
	fbo_f_depth_layers.is_fbo_complete();

	StE::LLR::GLUtils::query_gl_error(__FILE__, __LINE__);

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
// 			constexpr float rotation_factor = .05f;
// 			using StE::HID::PointerInput;
// 			auto pp = PointerInput::position();
// 			auto center = static_cast<glm::vec2>(rc.viewport_size())*.5f;
// 			PointerInput::set_position(center);
// 			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta.count() * rotation_factor;
// 			camera.pitch_and_yaw(diff_v.y, diff_v.x);
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
			glBindImageTexture(i, depth_layers, 0, false, i, GL_READ_WRITE, GL_R32I);
		transform.bind();
		auto mv = camera.view_matrix() * glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-100, -35, 0)), glm::vec3(.25, .25, .25));
		transform.set_uniform("view_model", mv);
		transform.set_uniform("trans_inverse_view_model", glm::transpose(glm::inverse(mv)));
		transform.set_uniform("projection", proj_mat);
		hall_model.render(transform);

// 		{
// 			auto model_mat = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-40, -35, -350)), glm::vec3(1.8,1.8,1.8));
// 			transform.set_uniform("model", model_mat);
// 			obj1_model.render();
// 		}
// 		 
// 		{
// 			auto model_mat = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(40, -35, -350)), glm::vec3(1.8, 1.8, 1.8));
// 			transform.set_uniform("model", model_mat);
// 			obj1_model.render();
// 		}

		//glMemoryBarrier(GL_ALL_BARRIER_BITS);
		rc.render_context().disable_depth_test();

		vao.bind();
		vao.enable_vertex_attrib_array(0);
		vao.enable_vertex_attrib_array(1);

		if (perform_ssao) {
			fbo_f_depth_layers.bind();
			fbo_f_depth_layers.set_rendering_targets({ { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } });
			gen_depth_layers.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, f_depth_layers);
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D_ARRAY, f_depth_layers);
			fbo_final1.bind();
			ssao.bind();
			ssao.set_uniform("steps", steps);
			ssao.set_uniform("proj_inv", glm::inverse(proj_mat));
			normal_output.bind(0);
			position_output.bind(1);
			noise.bind(2);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final2.bind();
			blur.bind();
			blur.set_uniform("blur_direction", glm::i32vec2(1, 0));
			occlusion_final1_output.bind(0);
			position_output.bind(1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			fbo_final1.bind();
			blur.bind();
			blur.set_uniform("blur_direction", glm::i32vec2(0, 1));
			occlusion_final2_output.bind(0);
			position_output.bind(1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		StE::LLR::FramebufferObject::unbind();
		deffered.bind();
		deffered.set_uniform("ssao", perform_ssao);
		deffered.set_uniform("view", camera.view_matrix());
		normal_output.bind(0);
		position_output.bind(1);
		occlusion_final1_output.bind(2);
		color_output.bind(3);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		StE::LLR::GLUtils::query_gl_error(__FILE__, __LINE__);

		return true;
	});
	 
	return 0;
}
