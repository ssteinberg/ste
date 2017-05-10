//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

namespace vk {

struct vk_rasterizer_op_descriptor {
	static constexpr auto default_front_face_orientation = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	bool discard_enable{ false };

	bool depth_bias_enable{ false };
	float depth_bias_const_factor{ .0f };
	float depth_bias_slope_factor{ .0f };

	VkCullModeFlags cull_mode{ VK_CULL_MODE_BACK_BIT };
	VkFrontFace front_face{ default_front_face_orientation };

	vk_rasterizer_op_descriptor() = default;
	vk_rasterizer_op_descriptor(VkCullModeFlags cull_mode,
								VkFrontFace front_face,
								bool depth_bias_enable,
								float depth_bias_const_factor,
								float depth_bias_slope_factor,
								bool discard = false)
		: discard_enable(discard),
		depth_bias_enable(depth_bias_enable),
		depth_bias_const_factor(depth_bias_const_factor), depth_bias_slope_factor(depth_bias_slope_factor),
		cull_mode(cull_mode), front_face(front_face)
	{}
};

}

}
}
