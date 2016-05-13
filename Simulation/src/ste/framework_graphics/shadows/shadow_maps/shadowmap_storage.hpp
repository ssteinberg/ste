// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "FramebufferObject.hpp"
#include "ShaderStorageBuffer.hpp"
#include "GLSLProgram.hpp"

#include "TextureCubeMapArray.hpp"
#include "Sampler.hpp"

#include "light_storage.hpp"

#include "reversed_perspective.hpp"

namespace StE {
namespace Graphics {

class shadowmap_storage {
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
	void update_transforms() {
		auto shadow_proj = reversed_infinite_perspective<float>(glm::half_pi<float>(), 1.f, shadow_proj_near_clip);
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
																										 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		set_count(max_active_lights_per_frame);

		shadow_depth_sampler.set_compare_mode(Core::TextureCompareMode::CompareToTextureDepth);
		shadow_depth_sampler.set_compare_func(Core::TextureCompareFunc::Greater);

		update_transforms();
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

	static void update_shader_shadow_proj_uniforms(Core::GLSLProgram *prog) {
		prog->set_uniform("shadow_proj23", shadow_proj_near_clip);
	}
};

}
}
