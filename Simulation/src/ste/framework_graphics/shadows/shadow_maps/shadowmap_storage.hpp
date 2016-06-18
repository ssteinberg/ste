// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "FramebufferObject.hpp"
#include "glsl_program.hpp"

#include "TextureCubeMapArray.hpp"
#include "Sampler.hpp"

#include "light_storage.hpp"

namespace StE {
namespace Graphics {

class shadowmap_storage {
	static constexpr unsigned default_map_size = 1024;
	static constexpr float shadow_proj_near_clip = 20.0f;

private:
	glm::uvec2 cube_size;
	std::unique_ptr<Core::TextureCubeMapArray> shadow_depth_cube_maps;
	Core::FramebufferObject shadow_depth_cube_map_fbo;

	Core::Sampler shadow_depth_sampler;

	signal<> storage_modified_signal;

public:
	shadowmap_storage(const StEngineControl &ctx,
					  const glm::uvec2 &cube_size = glm::uvec2(default_map_size)) : cube_size(cube_size),
																					shadow_depth_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear,
																										 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		set_count(max_active_lights_per_frame);

		shadow_depth_sampler.set_compare_mode(Core::TextureCompareMode::CompareToTextureDepth);
		shadow_depth_sampler.set_compare_func(Core::TextureCompareFunc::Greater);
	}

	void set_count(std::size_t size) {
		shadow_depth_cube_maps = std::make_unique<Core::TextureCubeMapArray>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec3{ cube_size.x, cube_size.y, size * 6 });
		shadow_depth_cube_map_fbo.depth_binding_point() = *shadow_depth_cube_maps;

		storage_modified_signal.emit();
	}
	auto count() const { return shadow_depth_cube_maps->get_layers(); }

	auto* get_fbo() const { return &shadow_depth_cube_map_fbo; }
	auto* get_cubemaps() const { return shadow_depth_cube_maps.get(); }
	auto& get_shadow_sampler() const { return shadow_depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
