// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "FramebufferObject.hpp"
#include "ShaderStorageBuffer.hpp"
#include "GLSLProgram.hpp"

#include "TextureCubeMapArray.hpp"
#include "Sampler.hpp"

#include "light_storage.hpp"

namespace StE {
namespace Graphics {

class shadowmap_storage {
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
	using proj_mat_buffer_type = Core::ShaderStorageBuffer<glm::mat4>;

	constexpr static unsigned default_map_size = 512;
	constexpr static float shadow_proj_near_clip = 20.0f;

private:
	glm::uvec2 cube_size;
	std::unique_ptr<Core::TextureCubeMapArray> shadow_depth_cube_maps;
	Core::FramebufferObject shadow_depth_cube_map_fbo;

	Core::Sampler shadow_depth_sampler;

	std::unique_ptr<proj_mat_buffer_type> shadow_projection_mats_buffer{ nullptr };

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void update_transforms(float far) {
		auto shadow_proj = glm::perspective(glm::half_pi<float>(), 1.f, shadow_proj_near_clip, 2.f * far);
		shadow_projection_mats_buffer = std::make_unique<proj_mat_buffer_type>(std::vector<glm::mat4>{
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f,-1.f, 0.f), glm::vec3(0.f, 0.f,-1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f, 1.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f,-1.f), glm::vec3(0.f,-1.f, 0.f))
		});
	}

public:
	shadowmap_storage(const StEngineControl &ctx,
					  const glm::uvec2 &cube_size = glm::uvec2(default_map_size)) : cube_size(cube_size),
																					shadow_depth_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear,
																										 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge, 16) {
		set_count(max_active_lights_per_frame);

		shadow_depth_sampler.set_compare_mode(Core::TextureCompareMode::CompareToTextureDepth);
		shadow_depth_sampler.set_compare_func(Core::TextureCompareFunc::Greater);

		update_transforms(ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float, float ffar) {
			update_transforms(ffar);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_count(std::size_t size) {
		shadow_depth_cube_maps = std::make_unique<Core::TextureCubeMapArray>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec3{ cube_size.x, cube_size.y, size * 6 });
		shadow_depth_cube_map_fbo.depth_binding_point() = *shadow_depth_cube_maps;
	}
	auto count() const { return shadow_depth_cube_maps->get_layers(); }

	auto* get_fbo() const { return &shadow_depth_cube_map_fbo; }
	auto* get_cubemaps() const { return shadow_depth_cube_maps.get(); }
	auto& get_shadow_sampler() const { return shadow_depth_sampler; }
	auto* get_shadow_projection_mats_buffer() const { return shadow_projection_mats_buffer.get(); }

	static void update_shader_shadow_proj_uniforms(Core::GLSLProgram *prog, float near, float far) {
		float f = 2.f * far;
		float n = shadow_proj_near_clip;
		float proj22 = -(f + n) / (n - f);
		float proj23 = -(2.f * f * n) / (n - f);

		prog->set_uniform("proj22", proj22);
		prog->set_uniform("proj23", proj23);
	}
};

}
}
