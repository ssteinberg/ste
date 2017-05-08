//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_pipeline_graphics.hpp>

#include <primitive_topology.hpp>
#include <rect.hpp>
#include <depth_range.hpp>

namespace ste {
namespace gl {

struct device_pipeline_graphics_configurations {
	rect viewport{ { .0f, .0f } };
	i32rect scissor{ { 0, 0 } };

	depth_range depth{ depth_range::one_to_zero() };

	primitive_topology topology{ primitive_topology::triangle_list };
	vk::vk_rasterizer_op_descriptor rasterizer_op;
	vk::vk_depth_op_descriptor depth_op;
	glm::vec4 blend_constants{};
};

}
}
