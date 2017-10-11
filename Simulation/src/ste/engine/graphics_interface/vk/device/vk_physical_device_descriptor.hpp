//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <vk_device_extension.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_physical_device_descriptor : public allow_type_decay<vk_physical_device_descriptor, VkPhysicalDevice> {
private:
	VkPhysicalDevice device{ nullptr };

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	vk_device_extensions device_extensions;

	lib::vector<VkQueueFamilyProperties> queue_family_properties;

private:
	/**
	*	@brief	Returns a list of available device extensions
	*/
	static auto enumerate_device_extensions(VkPhysicalDevice device) {
		std::uint32_t count;
		{
			// Count number of available extensions
			const vk_result res = vkEnumerateDeviceExtensionProperties(device,
																	   nullptr,
																	   &count,
																	   nullptr);
			if (!res) {
				throw vk::vk_exception(res);
			}
		}

		lib::vector<VkExtensionProperties> extensions;
		extensions.resize(count);
		{
			// Read extension list
			const vk_result res = vkEnumerateDeviceExtensionProperties(device,
																	   nullptr,
																	   &count,
																	   extensions.data());
			if (!res) {
				throw vk::vk_exception(res);
			}
		}

		// Prepare and return result
		lib::flat_set<vk_device_extension> list;
		list.reserve(count);
		for (auto &e : extensions)
			list.emplace(lib::string(e.extensionName), e.specVersion);

		return vk_device_extensions(std::move(list));
	}

public:
	vk_physical_device_descriptor() = default;
	vk_physical_device_descriptor(VkPhysicalDevice &&device)
		: device(std::move(device)),
		device_extensions(enumerate_device_extensions(this->device))
	{
		// Read devices' properties
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);
		vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

		std::uint32_t qcount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &qcount, nullptr);
		queue_family_properties.resize(qcount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &qcount, &queue_family_properties[0]);
	}

	vk_physical_device_descriptor(vk_physical_device_descriptor&&) = default;
	vk_physical_device_descriptor(const vk_physical_device_descriptor&) = default;
	vk_physical_device_descriptor &operator=(vk_physical_device_descriptor&&) = default;
	vk_physical_device_descriptor &operator=(const vk_physical_device_descriptor&) = default;

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

	auto& get() const { return device; }

	auto &get_properties() const { return properties; };
	auto &get_features() const { return features; };
	auto &get_memory_properties() const { return memory_properties; };
	auto &get_extensions() const { return device_extensions; };

	auto &get_queue_family_properties() const { return queue_family_properties; };
};

}

}
}
