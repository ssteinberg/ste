//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_result.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <vk_exception.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_extensions_proc_addr.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <optional.hpp>
#include <anchored.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_logical_device : public allow_type_decay<vk_logical_device<host_allocator>, VkDevice>, anchored {
private:
	vk_physical_device_descriptor physical_device;

	optional<VkDevice> logical_device;
	VkPhysicalDeviceFeatures requested_features;
	lib::vector<VkDeviceQueueCreateInfo> requested_queues;
	lib::vector<const char*> enabled_extensions;

	vk_extensions_proc_addr extensions_proc_addr;

public:
	vk_logical_device(const vk_physical_device_descriptor &physical_device,
					  const VkPhysicalDeviceFeatures &requested_features,
					  const lib::vector<VkDeviceQueueCreateInfo> &requested_queues,
					  const lib::vector<const char*> &device_extensions)
		: physical_device(physical_device), requested_features(requested_features),
		requested_queues(requested_queues), enabled_extensions(device_extensions)
	{
		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = nullptr;
		device_info.flags = 0;
		device_info.enabledExtensionCount = static_cast<std::uint32_t>(device_extensions.size());
		device_info.ppEnabledExtensionNames = &device_extensions[0];
		device_info.enabledLayerCount = 0;
		device_info.ppEnabledLayerNames = nullptr;
		device_info.queueCreateInfoCount = static_cast<std::uint32_t>(requested_queues.size());
		device_info.pQueueCreateInfos = &requested_queues[0];
		device_info.pEnabledFeatures = &requested_features;

		VkDevice device;
		const vk_result res = vkCreateDevice(physical_device, &device_info, &host_allocator::allocation_callbacks(), &device);
		if (!res) {
			throw vk_exception(res);
		}

		this->logical_device = device;

		// Get extensions' function pointers
		extensions_proc_addr = vk_extensions_proc_addr(device,
													   device_extensions);
	}

	~vk_logical_device() noexcept {
		if (logical_device) {
			wait_idle();
			vkDestroyDevice(logical_device.get(), &host_allocator::allocation_callbacks());
		}
		logical_device = none;
	}

	vk_logical_device(const vk_logical_device &) = delete;
	vk_logical_device &operator=(const vk_logical_device &) = delete;

	void wait_idle() const {
		vkDeviceWaitIdle(*this);
	}

	auto &get_physical_device_descriptor() const { return physical_device; }
	auto &get() const { return logical_device.get(); }
	auto &get_requested_features() const { return requested_features; }
	auto &get_extensions_func_pointers() const { return extensions_proc_addr; }
};

}

}
}
