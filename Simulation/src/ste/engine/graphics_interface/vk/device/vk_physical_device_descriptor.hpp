//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace vk {

struct vk_physical_device_descriptor {
	VkPhysicalDevice device{ nullptr };

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;

	lib::vector<VkQueueFamilyProperties> queue_family_properties;


	auto query_physical_device_format_properties(VkFormat format) const {
		VkFormatProperties out = {};
		vkGetPhysicalDeviceFormatProperties(device,
											format,
											&out);

		return out;
	}
};

}

}
}
