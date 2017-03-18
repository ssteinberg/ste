//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_instance.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_debug_report_callback.hpp>

#include <ste_gl_debug_callback.hpp>

#include <memory>

namespace StE {
namespace GL {

class ste_gl_context {
private:
	vk_instance vk;
	std::unique_ptr<vk_debug_report_callback> debug_report_handle;

private:
	static auto vk_instance_validation_layers() {
		return std::vector<const char*>{ "VK_LAYER_LUNARG_standard_validation" };
	}

	static auto create_vk_instance(const char* app_name,
								   unsigned app_version,
								   bool debug_context,
								   std::vector<const char*> instance_extensions,
								   std::vector<const char*> instance_layers) {
		std::uint32_t count;
		const char** extensions = glfwGetRequiredInstanceExtensions(&count);
		for (unsigned i = 0; i < count; ++i)
			instance_extensions.push_back(extensions[i]);

		// Add debug layers and extensions
		if (debug_context) {
			auto instance_validation_layers = vk_instance_validation_layers();
			instance_layers.insert(instance_layers.begin(), instance_validation_layers.begin(), instance_validation_layers.end());

			instance_extensions.push_back("VK_EXT_debug_report");
		}

		auto instance = GL::vk_instance(app_name, app_version,
										instance_extensions, instance_layers);
		return instance;
	}

private:
	static bool should_create_debug_context(const ste_gl_context_creation_parameters &parameters) {
		return parameters.debug_context ? parameters.debug_context.get() : false;
	}

	template <typename DebugCallback>
	void setup_vk_debug_callback() {
		debug_report_handle = std::make_unique<vk_debug_report_callback>(vk,
																		 this,
																		 [](VkDebugReportFlagsEXT        flags,
																			VkDebugReportObjectTypeEXT   objectType,
																			uint64_t                     object,
																			size_t                       location,
																			int32_t                      messageCode,
																			const char*                  pLayerPrefix,
																			const char*                  pMessage,
																			void*                        pUserData) -> VkBool32 {
			auto ptr = reinterpret_cast<const ste_gl_context*>(pUserData);
			DebugCallback()(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage, ptr);
			return VK_FALSE;
		});
	}

public:
	auto enumerate_physical_devices() {
		std::vector<vk_physical_device_descriptor> devices;

		// Read device count
		std::uint32_t count;
		vk_result res = vkEnumeratePhysicalDevices(vk, &count, nullptr);
		if (!res) {
			throw vk_exception(res);
		}
		if (!count) {
			throw vk_exception("vkEnumeratePhysicalDevices: No physical devices");
		}

		// Enumerate devices
		devices.resize(count);
		{
			std::unique_ptr<VkPhysicalDevice[]> t_devices_arr(new VkPhysicalDevice[count]);
			res = vkEnumeratePhysicalDevices(vk, &count, t_devices_arr.get());
			if (!res) {
				throw vk_exception(res);
			}

			for (unsigned i = 0; i < count; ++i)
				devices[i].device = t_devices_arr[i];
		}

		// Read devices' properties
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
									std::uint64_t min_device_memory) {
		std::vector<vk_physical_device_descriptor> conforming_devices;

		for (auto &d : enumerate_physical_devices()) {
			// Check device memory
			std::uint64_t total_device_local_heap_size = 0;
			for (std::uint32_t i = 0; i < d.memory_properties.memoryHeapCount; ++i)
				if ((d.memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
					total_device_local_heap_size += d.memory_properties.memoryHeaps[i].size;
			if (total_device_local_heap_size < min_device_memory)
				continue;

			// Check features
			bool satisfied_all_features = true;
			for (unsigned field_offset = 0; field_offset < sizeof(d.features) / sizeof(VkBool32); ++field_offset) {
				const auto *requested_field = reinterpret_cast<const VkBool32*>(&requested_features) + field_offset;
				const auto *device_field = reinterpret_cast<const VkBool32*>(&d.features) + field_offset;
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

public:
	template <typename DebugCallback = ste_gl_debug_callback>
	ste_gl_context(const ste_gl_context_creation_parameters &parameters)
		: vk(create_vk_instance(parameters.client_name, 
								parameters.client_version, 
								should_create_debug_context(parameters),
								parameters.additional_instance_extensions, 
								parameters.additional_instance_layers)) 
	{
		if (should_create_debug_context(parameters)) {
			setup_vk_debug_callback<DebugCallback>();
		}
	}
	~ste_gl_context() noexcept {}

	ste_gl_context(ste_gl_context &&) = default;
	ste_gl_context &operator=(ste_gl_context &&) = default;

	const auto& instance() const { return vk; }
};

}
}
