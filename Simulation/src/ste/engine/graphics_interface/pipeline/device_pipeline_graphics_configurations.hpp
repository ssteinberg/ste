//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_pipeline_graphics.hpp>

namespace StE {
namespace GL {

struct device_pipeline_graphics_configurations {
	VkViewport viewport{};
	VkRect2D scissor{};
	VkPrimitiveTopology topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	vk_rasterizer_op_descriptor rasterizer_op;
	vk_depth_op_descriptor depth_op;
	glm::vec4 blend_constants{};
};

}
}
