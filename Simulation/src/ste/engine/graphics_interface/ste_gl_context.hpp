//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_instance.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_debug_report_callback.hpp>

#include <ste_gl_context_creation_parameters.hpp>
#include <ste_gl_debug_callback.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

class ste_gl_context {
private:
	vk::vk_instance<> vk;
	lib::unique_ptr<vk::vk_debug_report_callback<>> debug_report_handle;

private:
	static lib::vector<const char*> vk_instance_validation_layers();
	static vk::vk_instance<> create_vk_instance(const char* app_name,
												unsigned app_version,
												bool debug_context,
												lib::vector<const char*> instance_extensions,
												lib::vector<const char*> instance_layers);

private:
	static bool should_create_debug_context(const ste_gl_context_creation_parameters &parameters) {
		return parameters.debug_context ? parameters.debug_context.get() : false;
	}

	template <typename DebugCallback>
	void setup_vk_debug_callback() {
		debug_report_handle = lib::allocate_unique<vk::vk_debug_report_callback<>>(vk,
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
		lib::vector<vk::vk_physical_device_descriptor> devices;

		// Read device count
		std::uint32_t count;
		vk::vk_result res = vkEnumeratePhysicalDevices(vk, &count, nullptr);
		if (!res) {
			throw vk::vk_exception(res);
		}
		if (!count) {
			throw vk::vk_exception("vkEnumeratePhysicalDevices: No physical devices");
		}

		// Enumerate devices
		devices.reserve(count);
		{
			auto t_devices_arr = lib::allocate_unique<VkPhysicalDevice[]>(count);
			res = vkEnumeratePhysicalDevices(vk, &count, t_devices_arr.get());
			if (!res) {
				throw vk::vk_exception(res);
			}

			for (unsigned i = 0; i < count; ++i)
				devices.emplace_back(std::move(t_devices_arr[i]));
		}

		return devices;
	}

	auto enumerate_physical_devices(const VkPhysicalDeviceFeatures &requested_features,
									byte_t min_device_memory) {
		lib::vector<vk::vk_physical_device_descriptor> conforming_devices;

		for (auto &d : enumerate_physical_devices()) {
			// Check device memory
			auto total_device_local_heap_size = 0_B;
			for (std::uint32_t i = 0; i < d.get_memory_properties().memoryHeapCount; ++i)
				if ((d.get_memory_properties().memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
					total_device_local_heap_size += byte_t(d.get_memory_properties().memoryHeaps[i].size);
			if (total_device_local_heap_size < min_device_memory)
				continue;

			// Check features
			bool satisfied_all_features = true;
			for (unsigned field_offset = 0; field_offset < sizeof(d.get_features()) / sizeof(VkBool32); ++field_offset) {
				const auto *requested_field = reinterpret_cast<const VkBool32*>(&requested_features) + field_offset;
				const auto *device_field = reinterpret_cast<const VkBool32*>(&d.get_features()) + field_offset;
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
