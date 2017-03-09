//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class vk_render_pass_subpass_dependency {
private:
	std::uint32_t src;
	std::uint32_t dst;
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;
	VkAccessFlags src_access;
	VkAccessFlags dst_access;
	VkDependencyFlags flags;

public:
	vk_render_pass_subpass_dependency(std::uint32_t src,
									  std::uint32_t dst,
									  VkPipelineStageFlags src_stage,
									  VkPipelineStageFlags dst_stage,
									  VkAccessFlags src_access,
									  VkAccessFlags dst_access,
									  VkDependencyFlags flags)
		: src(src),
		dst(dst),
		src_stage(src_stage),
		dst_stage(dst_stage),
		src_access(src_access),
		dst_access(dst_access),
		flags(flags)
	{}

	operator VkSubpassDependency() const {
		VkSubpassDependency dep = {};
		dep.srcSubpass = src;
		dep.dstSubpass = dst;
		dep.srcStageMask = src_stage;
		dep.dstStageMask = dst_stage;
		dep.srcAccessMask = src_access;
		dep.dstAccessMask = dst_access;
		dep.dependencyFlags = flags;

		return dep;
	}
};

}
}
