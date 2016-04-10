// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "TextureCubeMap.hpp"
#include "FramebufferObject.hpp"

namespace StE {
namespace Graphics {

class shadow_cubemap {
private:
	Core::TextureCubeMap shadow_depth_cube_map;
	Core::FramebufferObject shadow_depth_cube_map_fbo;

public:
	shadow_cubemap(const glm::uvec2 &cube_size = { 1024, 1024 }) : shadow_depth_cube_map(gli::format::FORMAT_D32_SFLOAT_PACK32, cube_size) {
		shadow_depth_cube_map_fbo.depth_binding_point() = shadow_depth_cube_map;
	}

	auto get_fbo() const { return &shadow_depth_cube_map_fbo; }
	auto get_cubemap() const { return &shadow_depth_cube_map; }
};

}
}
