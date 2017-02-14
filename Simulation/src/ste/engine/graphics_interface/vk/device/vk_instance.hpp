//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vk_physical_device_descriptor.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace StE {
namespace GL {

class vk_instance {
	static constexpr auto vk_api_version = VK_MAKE_VERSION(1, 0, 39);

private:
	VkInstance instance{ nullptr };

public:
	vk_instance(const char *app_name,
				unsigned app_version,
				const std::vector<const char*> &instance_extensions,
				const std::vector<const char*> &instance_layers) {
		VkApplicationInfo vk_app_info;
		vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vk_app_info.pNext = nullptr;
		vk_app_info.pApplicationName = app_name;
		vk_app_info.applicationVersion = app_version;
		vk_app_info.pEngineName = ste_name;
		vk_app_info.engineVersion = (ste_version_major << 16) + ste_version_minor;
		vk_app_info.apiVersion = vk_api_version;

		VkInstanceCreateInfo inst_info;
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = nullptr;
		inst_info.flags = 0;
		inst_info.enabledExtensionCount = instance_extensions.size();
		inst_info.ppEnabledExtensionNames = &instance_extensions[0];
		inst_info.enabledLayerCount = instance_layers.size();
		inst_info.ppEnabledLayerNames = &instance_layers[0];
		inst_info.pApplicationInfo = &vk_app_info;

		vk_result res = vkCreateInstance(&inst_info, nullptr, &instance);
		if (!res) {
			throw vk_exception(res);
		}
	}

	~vk_instance() {
		if (instance) {
			vkDestroyInstance(instance, nullptr);
		}
		instance = nullptr;
	}

	vk_instance(vk_instance &&) = default;

	auto enumerate_physical_devices() const {
		std::vector<vk_physical_device_descriptor> devices;

		std::uint32_t count;
		vk_result res = vkEnumeratePhysicalDevices(instance, &count, nullptr);
		if (!res) {
			throw vk_exception(res);
		}
		if (!count) {
			throw vk_exception("vkEnumeratePhysicalDevices: No physical devices");
		}

		devices.resize(count);
		{
			std::unique_ptr<VkPhysicalDevice[]> t_devices_arr(new VkPhysicalDevice[count]);
			res = vkEnumeratePhysicalDevices(instance, &count, t_devices_arr.get());
			if (!res) {
				throw vk_exception(res);
			}

			for (unsigned i = 0; i < count; ++i)
				devices[i].device = t_devices_arr[i];
		}

		for (auto &d : devices) {
			vkGetPhysicalDeviceProperties(d.device, &d.properties);
			vkGetPhysicalDeviceFeatures(d.device, &d.features);
			vkGetPhysicalDeviceMemoryProperties(d.device, &d.memory_properties);

			std::uint32_t qcount;
			vkGetPhysicalDeviceQueueFamilyProperties(d.device, &qcount, nullptr);
			d.queue_family_properties.resize(qcount);
			vkGetPhysicalDeviceQueueFamilyProperties(d.device, &qcount, &d.queue_family_properties[0]);
		}

		return devices;
	}

	auto enumerate_physical_devices(const VkPhysicalDeviceFeatures &requested_features,
									std::uint64_t min_device_memory) const {
		std::vector<vk_physical_device_descriptor> conforming_devices;

		for (auto &d : enumerate_physical_devices()) {
			// Check device memory
			std::uint64_t total_device_local_heap_size = 0;
			for (int i = 0; i < d.memory_properties.memoryHeapCount; ++i)
				if (d.memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT != 0)
					total_device_local_heap_size += d.memory_properties.memoryHeaps[i].size;
			if (total_device_local_heap_size < min_device_memory)
				continue;

			// Check features
			bool satisfied_all_features = true;
			for (unsigned field_offset = 0; field_offset < sizeof(d.features) / sizeof(VkBool32); ++field_offset) {
				const auto *requested_field = reinterpret_cast<const VkBool32*>(&requested_features) + field_offset;
				const auto *device_field	= reinterpret_cast<const VkBool32*>(&d.features) + field_offset;
				if (*requested_field && !*device_field) {
					satisfied_all_features = false;
					break;
				}
			}
			if (!satisfied_all_features)
				continue;;

			// Conforming device found
			conforming_devices.push_back(d);
		}

		return conforming_devices;
	}

	auto &get_instance() const { return instance; }
};

}
}
