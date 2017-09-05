//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_result.hpp>
#include <vk_exception.hpp>

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


	/**
	 *	@brief	Queries supported format properties.
	 */
	auto query_physical_device_format_properties(VkFormat format) const {
		VkFormatProperties out = {};
		vkGetPhysicalDeviceFormatProperties(device,
											format,
											&out);

		return out;
	}

	/**
	 *	@brief	In addition to the minimum capabilities, implementations may support additional capabilities for certain types of images. 
	 *			This queries the capabilities of image types.

	 */
	auto query_physical_device_image_properties(VkFormat format,
												VkImageType type,
												VkImageTiling tiling,
												VkImageUsageFlags usage,
												VkImageCreateFlags flags) const {
		VkImageFormatProperties out = {};
		const vk_result res = vkGetPhysicalDeviceImageFormatProperties(device,
																	   format,
																	   type,
																	   tiling,
																	   usage,
																	   flags,
																	   &out);
		if (!res) {
			// Don't raise an exception on not-supported error
			if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
				out = {};
				return out;
			}
			throw vk::vk_exception(res);
		}

		return out;
	}
};

}

}
}
