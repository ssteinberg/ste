//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class vk_push_constant_layout {
private:
	VkShaderStageFlags stage;
	std::uint32_t size;
	std::uint32_t offset;

public:
	vk_push_constant_layout(const VkShaderStageFlags &stage,
							std::uint32_t size,
							std::uint32_t offset = 0)
		: stage(stage), size(size), offset(offset)
	{}

	vk_push_constant_layout(vk_push_constant_layout &&) = default;
	vk_push_constant_layout &operator=(vk_push_constant_layout &&) = default;
	vk_push_constant_layout(const vk_push_constant_layout &) = default;
	vk_push_constant_layout &operator=(const vk_push_constant_layout &) = default;

	auto get_layout() const {
		VkPushConstantRange layout = {};
		layout.stageFlags = stage;
		layout.size = size;
		layout.offset = offset;

		return layout;
	}

	operator VkPushConstantRange() const { return get_layout(); }
};

}
}
