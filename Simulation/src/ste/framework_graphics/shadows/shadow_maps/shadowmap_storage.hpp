// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "signal.hpp"

#include "FramebufferObject.hpp"

#include "TextureCubeMapArray.hpp"
#include "Texture2DArray.hpp"
#include "Sampler.hpp"

#include "light_storage.hpp"
#include "light_cascade_descriptor.hpp"

namespace StE {
namespace Graphics {

class shadowmap_storage {
	static constexpr unsigned default_map_size = 1024;
	static constexpr unsigned default_directional_map_size = 1024;

private:
	glm::uvec2 cube_size;
	std::unique_ptr<Core::TextureCubeMapArray> shadow_depth_cube_maps;
	Core::FramebufferObject shadow_depth_cube_map_fbo;

	glm::uvec2 directional_map_size;
	std::unique_ptr<Core::Texture2DArray> directional_shadow_maps;
	Core::FramebufferObject directional_shadow_maps_fbo;

	Core::Sampler shadow_depth_sampler;

	signal<> storage_modified_signal;

public:
	shadowmap_storage(const StEngineControl &ctx,
					  const glm::uvec2 &directional_map_size = glm::uvec2(default_directional_map_size),
					  const glm::uvec2 &cube_size = glm::uvec2(default_map_size)) : cube_size(cube_size),
																					directional_map_size(directional_map_size),
																					shadow_depth_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear,
																										 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		set_cube_count(max_active_lights_per_frame);
		set_directional_maps_count(max_active_directional_lights_per_frame);

		shadow_depth_sampler.set_compare_mode(Core::TextureCompareMode::CompareToTextureDepth);
		shadow_depth_sampler.set_compare_func(Core::TextureCompareFunc::Greater);
	}

	void set_cube_count(std::size_t size) {
		shadow_depth_cube_maps = std::make_unique<Core::TextureCubeMapArray>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec3{ cube_size.x, cube_size.y, size * 6 });
		shadow_depth_cube_map_fbo.depth_binding_point() = *shadow_depth_cube_maps;

		storage_modified_signal.emit();
	}
	auto get_cube_count() const { return shadow_depth_cube_maps->get_layers(); }

	void set_directional_maps_count(std::size_t size) {
		directional_shadow_maps = std::make_unique<Core::Texture2DArray>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec3{ directional_map_size.x, directional_map_size.y, size * directional_light_cascades });
		directional_shadow_maps_fbo.depth_binding_point() = *directional_shadow_maps;

		storage_modified_signal.emit();
	}
	auto get_directional_maps_count() const { return directional_shadow_maps->get_layers() / directional_light_cascades; }

	auto* get_cube_fbo() const { return &shadow_depth_cube_map_fbo; }
	auto* get_cubemaps() const { return shadow_depth_cube_maps.get(); }

	auto* get_directional_maps_fbo() const { return &directional_shadow_maps_fbo; }
	auto* get_directional_maps() const { return directional_shadow_maps.get(); }

	auto& get_shadow_sampler() const { return shadow_depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
