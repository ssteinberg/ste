//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace vk {

namespace _internal {

struct vk_ext_debug_marker {
	bool enabled{ false };
	PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT;
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
	PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT;
	PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT;
	PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT;
};

struct vk_khr_get_memory_requirements2 {
	bool enabled{ false };
	PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR;
	PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR;
	PFN_vkGetImageSparseMemoryRequirements2KHR vkGetImageSparseMemoryRequirements2KHR;
};

struct vk_khr_bind_memory2 {
	bool enabled{ false };
	PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR;
	PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR;
};

}

class vk_extensions_proc_addr {
private:
	_internal::vk_ext_debug_marker ext_debug_marker;
	_internal::vk_khr_get_memory_requirements2 khr_get_memory_requirements2;
	_internal::vk_khr_bind_memory2 khr_bind_memory2;

public:
	vk_extensions_proc_addr() = default;
	vk_extensions_proc_addr(const VkDevice &device,
							const lib::vector<const char*> &device_extensions) {
		// VK_EXT_debug_marker
		ext_debug_marker.enabled = std::find(device_extensions.begin(), device_extensions.end(), VK_EXT_DEBUG_MARKER_EXTENSION_NAME) != device_extensions.end();

		if (ext_debug_marker.enabled) {
			ext_debug_marker.vkDebugMarkerSetObjectTagEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
			ext_debug_marker.vkDebugMarkerSetObjectNameEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
			ext_debug_marker.vkCmdDebugMarkerBeginEXT = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
			ext_debug_marker.vkCmdDebugMarkerEndEXT = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
			ext_debug_marker.vkCmdDebugMarkerInsertEXT = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));
		}

		// VK_KHR_get_memory_requirements2
		khr_get_memory_requirements2.enabled = std::find(device_extensions.begin(), device_extensions.end(), VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) != device_extensions.end();

		if (khr_get_memory_requirements2.enabled) {
			khr_get_memory_requirements2.vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2KHR"));
			khr_get_memory_requirements2.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2KHR"));
			khr_get_memory_requirements2.vkGetImageSparseMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements2KHR>(vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2KHR"));
		}

		// VK_KHR_bind_memory2
		khr_bind_memory2.enabled = std::find(device_extensions.begin(), device_extensions.end(), VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) != device_extensions.end();

		if (khr_bind_memory2.enabled) {
			khr_bind_memory2.vkBindBufferMemory2KHR = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR"));
			khr_bind_memory2.vkBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR"));
		}
	}

	auto& debug_marker() const { return ext_debug_marker; }
	auto& get_memory_requirements2() const { return khr_get_memory_requirements2; }
	auto& get_bind_memory2() const { return khr_bind_memory2; }
};

}

}
}
