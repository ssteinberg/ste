//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_depth_op_descriptor.hpp>

#include <compare_op.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class depth_operation : public allow_type_decay<depth_operation, vk::vk_depth_op_descriptor> {
private:
	vk::vk_depth_op_descriptor op;

public:
	depth_operation() = default;
	depth_operation(compare_op depth_compare_op,
					bool enable_depth_write = true)
		: op(static_cast<VkCompareOp>(depth_compare_op), enable_depth_write) {}

	auto &get() const { return op; }
};

}
}
