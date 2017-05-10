//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <depth_operation.hpp>
#include <rasterizer_operation.hpp>
#include <primitive_topology.hpp>

namespace ste {
namespace gl {

struct device_pipeline_graphics_configurations {
	primitive_topology topology{ primitive_topology::triangle_list };

	depth_operation depth_op;
	rasterizer_operation rasterizer_op;
	glm::vec4 blend_constants{};
};

}
}
