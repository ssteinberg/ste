//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class pipeline_dynamic_state : std::uint32_t {
	viewport = VK_DYNAMIC_STATE_VIEWPORT,
	scissor = VK_DYNAMIC_STATE_SCISSOR,
	line_width = VK_DYNAMIC_STATE_LINE_WIDTH,
	depth_bias = VK_DYNAMIC_STATE_DEPTH_BIAS,
	blend_constants = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
	depth_bounds = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
	stencil_compare_mask = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
	stencil_write_mask = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
	stencil_reference = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
	viewport_w_scaling_nv = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
	discard_rectangle_ext = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
};

}
}
