//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

namespace vk {

struct vk_depth_op_descriptor {
	bool depth_test_enable{ false };
	bool depth_write_enable{ false };
	VkCompareOp depth_compare_op{ VK_COMPARE_OP_GREATER };

	vk_depth_op_descriptor() = default;
	vk_depth_op_descriptor(VkCompareOp depth_compare_op,
						   bool write = true)
		: depth_test_enable(true), depth_write_enable(write), depth_compare_op(depth_compare_op) {}
};

}

}
}
