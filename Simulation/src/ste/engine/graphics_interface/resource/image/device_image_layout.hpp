//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_image_initial_layout.hpp>

#include <atomic>

namespace StE {
namespace GL {

struct device_image_layout {
	std::atomic<VkImageLayout> layout;

	device_image_layout() = delete;
	device_image_layout(const vk_image_initial_layout &layout)
		: layout(layout == vk_image_initial_layout::preinitialized ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED)
	{}
	device_image_layout(const VkImageLayout &layout)
		: layout(layout)
	{}

	device_image_layout(device_image_layout &&o) noexcept
		: layout(o.layout.load()) {}
	device_image_layout &operator=(device_image_layout &&o) noexcept {
		layout.store(o.layout);
		return *this;
	}
};

}
}
