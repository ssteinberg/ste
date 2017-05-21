//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

#include <cstring>

namespace ste {
namespace gl {

namespace vk {

struct vk_blend_op_descriptor {
	VkColorComponentFlags write_mask{ VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT };

	bool blend_enable{ false };
	VkBlendFactor src_color{ VK_BLEND_FACTOR_SRC_COLOR };
	VkBlendFactor dst_color{ VK_BLEND_FACTOR_DST_COLOR };
	VkBlendOp color_op{ VK_BLEND_OP_ADD };
	VkBlendFactor src_alpha{ VK_BLEND_FACTOR_SRC_ALPHA };
	VkBlendFactor dst_alpha{ VK_BLEND_FACTOR_DST_ALPHA };
	VkBlendOp alpha_op{ VK_BLEND_OP_ADD };

	vk_blend_op_descriptor() = default;
	vk_blend_op_descriptor(const VkColorComponentFlags &write_mask,
						   const VkBlendFactor &src_color,
						   const VkBlendFactor &dst_color,
						   const VkBlendOp &color_op,
						   const VkBlendFactor &src_alpha,
						   const VkBlendFactor &dst_alpha,
						   const VkBlendOp &alpha_op)
		: write_mask(write_mask),
		blend_enable(true),
		src_color(src_color),
		dst_color(dst_color),
		color_op(color_op),
		src_alpha(src_alpha),
		dst_alpha(dst_alpha),
		alpha_op(alpha_op)
	{}

	bool operator==(const vk_blend_op_descriptor &rhs) const {
		return std::memcmp(this, &rhs, sizeof(rhs)) == 0;
	}
	bool operator!=(const vk_blend_op_descriptor &rhs) const {
		return !(*this == rhs);
	}

	operator VkPipelineColorBlendAttachmentState() const {
		VkPipelineColorBlendAttachmentState state = {};

		state.colorWriteMask = write_mask;
		state.blendEnable = blend_enable;
		state.colorBlendOp = color_op;
		state.alphaBlendOp = alpha_op;
		state.srcColorBlendFactor = src_color;
		state.dstColorBlendFactor = dst_color;
		state.srcAlphaBlendFactor = src_alpha;
		state.dstAlphaBlendFactor = dst_alpha;

		return state;
	}
};

}

}
}
