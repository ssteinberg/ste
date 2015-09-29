
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
#include "Keyboard.h"
#include "Pointer.h"
#include "StEngineControl.h"
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
#include "PixelBufferObject.h"
#include "AtomicCounterBuffer.h"
#include "concurrent_unordered_map.h"

#include "tbb/concurrent_hash_map.h"
#include <chrono>
#include <random>

using namespace tbb;

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

	static constexpr float read_ratio = .9f;

	StE::concurrent_unordered_map<int, std::string> map;
	concurrent_hash_map<int, std::string> tbb_map;

	auto time = std::chrono::high_resolution_clock::now();
	for (int j = 0; j < 100; ++j) {
		tbb_map.insert({ j, std::string("temp") + std::to_string(j) });
	}
	std::chrono::duration<float> tbb_delta = std::chrono::high_resolution_clock::now() - time;
	ste_log() << "TBB creation time: " << tbb_delta.count() << "sec" << std::endl;

	time = std::chrono::high_resolution_clock::now();
	for (int j = 0; j < 100; ++j) {
		map.emplace(j, std::string("temp") + std::to_string(j));
	}
	std::chrono::duration<float> ste_delta = std::chrono::high_resolution_clock::now() - time;
	ste_log() << "StE creation time: " << ste_delta.count() << "sec" << std::endl;
 
 	time = std::chrono::high_resolution_clock::now();
 	{
		std::thread t1([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					tbb_map.insert({ -1, "test1" });
					tbb_map.insert({ ((j + i) * 101) % 98710, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143896t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 1801) % 90, "fset4weeeeeeebryeyhregfdhg r32r34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 10) % 910, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr14389hgdf 5 566t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 31) % 865710, "fset4weeeeeeebyt54t  481tr434r 4 4wr" });
					tbb_map.erase(((j + i) * 42) % 231320);
					tbb_map.erase(((j + i) * 99) % 2310);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					concurrent_hash_map<int, std::string>::accessor result;
					tbb_map.find(result,((j + i) * 48) % 342);
					tbb_map.find(result,((j + i) * 54) % 63);
					tbb_map.find(result,((j + i) * 12) % 654);
					tbb_map.find(result,((j + i) * 87) % 1345340);
					tbb_map.find(result,((j + i) * 54) % 63);
					tbb_map.find(result,-1);
				}
			}
		});
		std::thread t2([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					tbb_map.insert({ ((j + i) * 546) % 54352, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 43) % 43, "fset4weeeeeeebryeyhregfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr" });
					tbb_map.erase(((j + i) * 124) % 2313320);
					tbb_map.insert({ ((j + i) * 3) % 434, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr1434389hgdf 5 566t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 564) % 12325, "fset4weeeeeeebyt54t  481trfd434r 4 4wr" });
					tbb_map.insert({ -1, "test2" });
					tbb_map.erase(((j + i) * 399) % 10);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					concurrent_hash_map<int, std::string>::accessor result;
					tbb_map.find(result,((j + i) * 4) % 786);
					tbb_map.find(result,((j + i) * 534) % 635);
					tbb_map.find(result,((j + i) * 4312) % 65363);
					tbb_map.find(result,((j + i) * 7) % 6);
					tbb_map.find(result,((j + i) * 534) % 64343443);
					tbb_map.find(result,-1);
				}
			}
		});
		std::thread t3([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					tbb_map.erase(((j + i) * 545) % 54);
					tbb_map.insert({ ((j + i) * 324) % 76, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 4) % 34, "fset4weeeeeeebryeyhregfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr" });
					tbb_map.erase(((j + i) * 34) % 1234);
					tbb_map.insert({ -1, "test3" });
					tbb_map.insert({ ((j + i) * 3) % 434, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr1434389hgdf 5 566t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 564) % 12325, "fset4weeeeeeebyt54t  481trfd434r 4 4wr" });
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					concurrent_hash_map<int, std::string>::accessor result;
					tbb_map.find(result,((j + i) * 4) % 435);
					tbb_map.find(result,((j + i) * 54) % 43);
					tbb_map.find(result,((j + i) * 43) % 12);
					tbb_map.find(result,((j + i) * 54) % 6);
					tbb_map.find(result,((j + i) * 8) % 4565);
					tbb_map.find(result,-1);
				}
			}
		});
		std::thread t4([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					tbb_map.insert({ ((j + i) * 556) % 54352, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr" });
					tbb_map.insert({ ((j + i) * 43) % 43, "fset4weeeeeeebryeyhre54gfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr" });
					tbb_map.erase(((j + i) * 124) % 231320);
					tbb_map.erase(((j + i) * 43) % 140);
					tbb_map.erase(((j + i) * 3399) % 1234);
					tbb_map.insert({ -1, "test4" });
					tbb_map.erase(((j + i) * 1) % 43);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					concurrent_hash_map<int, std::string>::accessor result;
					tbb_map.find(result,((j + i) * 14) % 78236);
					tbb_map.find(result,((j + i) * 534) % 65);
					tbb_map.find(result,((j + i) * 243) % 12);
					tbb_map.find(result,((j + i) * 54) % 4);
					tbb_map.find(result,((j + i) * 94) % 63);
					tbb_map.find(result,-1);
				}
			}
		});
 
 
 		t1.join();
 		t2.join();
 		t3.join();
 		t4.join();
 		std::chrono::duration<float> tbb_delta = std::chrono::high_resolution_clock::now() - time;
 		ste_log() << "TBB time: " << tbb_delta.count() << "sec" << std::endl;
 	}

	time = std::chrono::high_resolution_clock::now();
	{
		std::thread t1([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					map.emplace(-1, "test1");
					map.emplace(((j + i) * 101) % 98710, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143896t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 1801) % 90, "fset4weeeeeeebryeyhregfdhg r32r34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 10) % 910, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr14389hgdf 5 566t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 31) % 865710, "fset4weeeeeeebyt54t  481tr434r 4 4wr");
					map.remove(((j + i) * 42) % 231320);
					map.remove(((j + i) * 99) % 2310);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					auto ret1 = map.try_get(((j + i) * 48) % 342);
					auto ret2 = map.try_get(((j + i) * 54) % 63);
					auto ret3 = map.try_get(((j + i) * 12) % 654);
					auto ret4 = map.try_get(((j + i) * 87) % 1345340);
					auto ret5 = map.try_get(((j + i) * 54) % 63);
					auto ret = map.try_get(-1);
					if (ret.is_valid())
						assert(ret->find("test") != std::string::npos);
				}
			}
		});
		std::thread t2([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					map.emplace(((j + i) * 546) % 54352, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 43) % 43, "fset4weeeeeeebryeyhregfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr");
					map.remove(((j + i) * 124) % 2313320);
					map.emplace(((j + i) * 3) % 434, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr1434389hgdf 5 566t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 564) % 12325, "fset4weeeeeeebyt54t  481trfd434r 4 4wr");
					map.emplace(-1, "test2");
					map.remove(((j + i) * 399) % 10);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					auto ret1 = map.try_get(((j + i) * 4) % 786);
					auto ret2 = map.try_get(((j + i) * 534) % 635);
					auto ret3 = map.try_get(((j + i) * 4312) % 65363);
					auto ret4 = map.try_get(((j + i) * 7) % 6);
					auto ret5 = map.try_get(((j + i) * 534) % 64343443);
					auto ret = map.try_get(-1);
					if (ret.is_valid())
						assert(ret->find("test") != std::string::npos);
				}
			}
		});
		std::thread t3([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					map.remove(((j + i) * 545) % 54);
					map.emplace(((j + i) * 324) % 76, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 4) % 34, "fset4weeeeeeebryeyhregfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr");
					map.remove(((j + i) * 34) % 1234);
					map.emplace(-1, "test3");
					map.emplace(((j + i) * 3) % 434, "fset4weeeeeeebyt54t 4w39r r93 gd30ur83429  yor  r34rt42wrt42tr1434389hgdf 5 566t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 564) % 12325, "fset4weeeeeeebyt54t  481trfd434r 4 4wr");
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					auto ret1 = map.try_get(((j + i) * 4) % 435);
					auto ret2 = map.try_get(((j + i) * 54) % 43);
					auto ret3 = map.try_get(((j + i) * 43) % 12);
					auto ret4 = map.try_get(((j + i) * 54) % 6);
					auto ret5 = map.try_get(((j + i) * 8) % 4565);
					auto ret = map.try_get(-1);
					if (ret.is_valid())
						assert(ret->find("test") != std::string::npos);
				}
			}
		});
		std::thread t4([&]() {
			for (int i = 0; i < 1000000; ++i) {
				std::string str;
				for (int j = 0; j < 100 * (1 - read_ratio); ++j) {
					map.emplace(((j + i) * 556) % 54352, "fset4weeeeeeebyt54t 4w39r r93 30ur83429 r32r34r yor  r34rt42wrt42tr143fe896t4 481tr434r 4 4wr");
					map.emplace(((j + i) * 43) % 43, "fset4weeeeeeebryeyhre54gfdhg r32rer34r yor  r34rt42wrtgfdfd42tr143896t4 481tr434r 4 4wr");
					map.remove(((j + i) * 124) % 231320);
					map.remove(((j + i) * 43) % 140);
					map.remove(((j + i) * 3399) % 1234);
					map.emplace(-1, "test4");
					map.remove(((j + i) * 1) % 43);
				}
				for (int j = 0; j < 100 * read_ratio; ++j) {
					auto ret1 = map.try_get(((j + i) * 14) % 78236);
					auto ret2 = map.try_get(((j + i) * 534) % 65);
					auto ret3 = map.try_get(((j + i) * 243) % 12);
					auto ret4 = map.try_get(((j + i) * 54) % 4);
					auto ret5 = map.try_get(((j + i) * 94) % 63);
					auto ret = map.try_get(-1);
					if (ret.is_valid())
						assert(ret->find("test") != std::string::npos);
				}
			}
		});

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		std::chrono::duration<float> ste_delta = std::chrono::high_resolution_clock::now() - time;
		ste_log() << "StE time: " << ste_delta.count() << "sec" << std::endl;
	}

	return true;

	ste_log() << "Simulation running";

	constexpr float w = 1400, h = 900;
	constexpr int max_steps = 8;
	constexpr int depth_layers_count = 3;
	constexpr float clip_far = 1000.f;
	constexpr float clip_near = 1.f;

	StE::StEngineControl rc;
	rc.init_render_context("Shlomi Steinberg - Simulation", { w, h });
	rc.set_clipping_planes(clip_near, clip_far);
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
	StE::LLR::Texture2D noise(tex);
	noise.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	noise.set_mag_filter(StE::LLR::TextureFiltering::Nearest);

	StE::LLR::RenderTarget depth_output(gli::format::FORMAT_D24_UNORM, StE::LLR::Texture2D::size_type(w, h));
	StE::LLR::Texture2D normal_output(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	normal_output.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	normal_output.set_mag_filter(StE::LLR::TextureFiltering::Nearest);
	StE::LLR::Texture2D position_output(gli::format::FORMAT_RGB32_SFLOAT, StE::LLR::Texture2D::size_type(w, h), 1);
	StE::LLR::Texture2D color_output(gli::format::FORMAT_RGB8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	color_output.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	color_output.set_mag_filter(StE::LLR::TextureFiltering::Nearest);
	StE::LLR::FramebufferObject fbo;
	fbo.depth_binding_point() = depth_output;
	fbo[0] = position_output[0];
	fbo[1] = color_output[0];
	fbo[2] = normal_output[0];
	1_color_idx = fbo[0];
	0_color_idx = fbo[1];
	2_color_idx = fbo[2];

	StE::LLR::Texture2D occlusion_final1_output(gli::format::FORMAT_R8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	occlusion_final1_output.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	occlusion_final1_output.set_mag_filter(StE::LLR::TextureFiltering::Nearest);
	occlusion_final1_output.set_wrap_s(StE::LLR::TextureWrapMode::ClampToEdge);
	occlusion_final1_output.set_wrap_t(StE::LLR::TextureWrapMode::ClampToEdge);
	StE::LLR::FramebufferObject fbo_final1;
	fbo_final1[0] = occlusion_final1_output[0];

	StE::LLR::Texture2D occlusion_final2_output(gli::format::FORMAT_R8_UNORM, StE::LLR::Texture2D::size_type(w, h), 1);
	occlusion_final2_output.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	occlusion_final2_output.set_mag_filter(StE::LLR::TextureFiltering::Nearest);
	occlusion_final2_output.set_wrap_s(StE::LLR::TextureWrapMode::ClampToEdge);
	occlusion_final2_output.set_wrap_t(StE::LLR::TextureWrapMode::ClampToEdge);
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

	StE::LLR::Texture2DArray depth_layers(gli::format::FORMAT_R32_UINT, StE::LLR::Texture2DArray::size_type(w, h, depth_layers_count), 1);
	depth_layers.set_mag_filter(StE::LLR::TextureFiltering::Nearest);
	depth_layers.set_min_filter(StE::LLR::TextureFiltering::Nearest);
	depth_layers.set_wrap_s(StE::LLR::TextureWrapMode::ClampToEdge);
	depth_layers.set_wrap_t(StE::LLR::TextureWrapMode::ClampToEdge);
	StE::LLR::FramebufferObject fbo_depth_layers;
	for (int i = 0; i < depth_layers_count; ++i)
		fbo_depth_layers[i] = depth_layers[0][i].with_format(gli::format::FORMAT_R32_SINT);

	StE::LLR::Texture2DArray f_depth_layers(gli::format::FORMAT_R32_SFLOAT, StE::LLR::Texture2DArray::size_type(w, h, depth_layers_count), max_steps);
	f_depth_layers.set_min_filter(StE::LLR::TextureFiltering::Linear);
	f_depth_layers.set_mipmap_filter(StE::LLR::TextureFiltering::Nearest);
	f_depth_layers.set_wrap_s(StE::LLR::TextureWrapMode::ClampToEdge);
	f_depth_layers.set_wrap_t(StE::LLR::TextureWrapMode::ClampToEdge);
	StE::LLR::FramebufferObject fbo_f_depth_layers;
	for (int i = 0; i < depth_layers_count; ++i)
		fbo_f_depth_layers[i] = f_depth_layers[0][i];

 	for (int i = 0; i < depth_layers_count; ++i)
		depth_layers[0][i].with_format(gli::format::FORMAT_R32_SINT).bind(image_layout_binding(i));

	ste_log_query_and_log_gl_errors();

	StE::Resource::Model hall_model;
	hall_model.load_model(R"(data\models\sponza.obj)");

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

			rc.render_context().default_framebuffer->blit_to(fbo,size,size);
			fbo[0].read_pixels(tex.data(), 3 * size.x * size.y);

			StE::Resource::SurfaceIO::write_surface_2d(tex, R"(D:\a.png)");
		};
	});
	rc.hid_signal_keyboard().connect(keyboard_listner);

	rc.set_pointer_hidden(true);

	// Run main loop
	rc.run_loop([&]() {
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
			auto center = static_cast<glm::vec2>(rc.get_viewport_size())*.5f;
			rc.set_pointer_position(static_cast<glm::ivec2>(center));
			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta * rotation_factor;
			camera.pitch_and_yaw(-diff_v.y, diff_v.x);
		}

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
		hall_model.render(transform);

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

		rc.render_context().default_framebuffer->bind();
		fbo_final1.unbind();
		deffered.bind();
		deffered.set_uniform("ssao", perform_ssao);
		deffered.set_uniform("view", camera.view_matrix());
		0_sampler_idx = normal_output;
		1_sampler_idx = position_output;
		2_sampler_idx = occlusion_final1_output;
		3_sampler_idx = color_output;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		ste_log_query_and_log_gl_errors();

		return running;
	});
	 
	return 0;
}
