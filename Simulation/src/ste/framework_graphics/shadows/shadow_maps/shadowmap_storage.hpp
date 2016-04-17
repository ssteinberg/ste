// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "TextureCubeMapArray.hpp"
#include "FramebufferObject.hpp"

namespace StE {
namespace Graphics {

class shadowmap_storage {
private:
	glm::uvec2 cube_size;
	std::unique_ptr<Core::TextureCubeMapArray> shadow_depth_cube_maps;
	Core::FramebufferObject shadow_depth_cube_map_fbo;

public:
	shadowmap_storage(const glm::uvec2 &cube_size = { 1024, 1024 }) : cube_size(cube_size) {
		set_count(2);
	}

	void set_count(std::size_t size) {
		shadow_depth_cube_maps = std::make_unique<Core::TextureCubeMapArray>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec3{ cube_size.x, cube_size.y, size * 6 });
		shadow_depth_cube_map_fbo.depth_binding_point() = *shadow_depth_cube_maps;
	}
	auto count() const { return shadow_depth_cube_maps->get_layers(); }

	auto* get_fbo() const { return &shadow_depth_cube_map_fbo; }
	auto* get_cubemaps() const { return shadow_depth_cube_maps.get(); }
};

}
}
