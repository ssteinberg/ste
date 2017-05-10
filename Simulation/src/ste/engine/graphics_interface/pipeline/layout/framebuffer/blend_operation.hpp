//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_blend_op_descriptor.hpp>

#include <blend_factor.hpp>
#include <blend_op.hpp>
#include <color_component.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class blend_operation : public allow_type_decay<blend_operation, vk::vk_blend_op_descriptor> {
private:
	vk::vk_blend_op_descriptor op;

public:
	blend_operation() = default;
	blend_operation(const color_component &write_mask)
		: op(static_cast<VkColorComponentFlags>(write_mask),
			 VK_BLEND_FACTOR_SRC_COLOR,
			 VK_BLEND_FACTOR_DST_COLOR,
			 VK_BLEND_OP_ADD,
			 VK_BLEND_FACTOR_SRC_ALPHA,
			 VK_BLEND_FACTOR_DST_ALPHA,
			 VK_BLEND_OP_ADD) 
	{}
	blend_operation(const blend_factor &src_color,
					const blend_factor &dst_color,
					const blend_op &color_op,
					const blend_factor &src_alpha,
					const blend_factor &dst_alpha,
					const blend_op &alpha_op)
		: op(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			 static_cast<VkBlendFactor>(src_color),
			 static_cast<VkBlendFactor>(dst_color),
			 static_cast<VkBlendOp>(color_op),
			 static_cast<VkBlendFactor>(src_alpha),
			 static_cast<VkBlendFactor>(dst_alpha),
			 static_cast<VkBlendOp>(alpha_op))
	{}
	blend_operation(const blend_factor &src,
					const blend_factor &dst,
					const blend_op &op)
		: op(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			 static_cast<VkBlendFactor>(src),
			 static_cast<VkBlendFactor>(dst),
			 static_cast<VkBlendOp>(op),
			 static_cast<VkBlendFactor>(src),
			 static_cast<VkBlendFactor>(dst),
			 static_cast<VkBlendOp>(op))
	{}
	blend_operation(const color_component &write_mask,
					const blend_factor &src_color,
					const blend_factor &dst_color,
					const blend_op &color_op,
					const blend_factor &src_alpha,
					const blend_factor &dst_alpha,
					const blend_op &alpha_op)
		: op(static_cast<VkColorComponentFlags>(write_mask),
			 static_cast<VkBlendFactor>(src_color),
			 static_cast<VkBlendFactor>(dst_color),
			 static_cast<VkBlendOp>(color_op),
			 static_cast<VkBlendFactor>(src_alpha),
			 static_cast<VkBlendFactor>(dst_alpha),
			 static_cast<VkBlendOp>(alpha_op))
	{}

	bool operator==(const blend_operation &rhs) const {
		return op == rhs.op;
	}
	bool operator!=(const blend_operation &rhs) const {
		return !(*this == rhs);
	}

	auto &get() const { return op; }
};

}
}
