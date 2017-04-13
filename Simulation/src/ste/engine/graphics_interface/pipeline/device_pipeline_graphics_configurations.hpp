//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_pipeline_graphics.hpp>

#include <vector>

namespace StE {
namespace GL {

struct device_pipeline_graphics_configurations {
	VkViewport viewport;
	VkRect2D scissor;
	std::vector<vk_pipeline_graphics::vertex_input_descriptor> vertex_attributes;
	VkPrimitiveTopology topology;
	vk_rasterizer_op_descriptor rasterizer_op;
	vk_depth_op_descriptor depth_op;
	std::vector<vk_blend_op_descriptor> attachment_blend_op;
	glm::vec4 blend_constants;
};

}
}
