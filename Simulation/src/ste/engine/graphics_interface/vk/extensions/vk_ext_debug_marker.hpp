//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_logical_device.hpp>

#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

namespace vk {

/**
 *	@brief	Attaches a debug marker marker, that consists of a string, to a Vulkan object.
 *			Does nothing if extension VK_EXT_debug_marker isn't enabled by device.
 */
template <typename T, typename A>
void vk_debug_marker_set_object_name(const vk_logical_device<A> &device,
									 const T &t,
									 VkDebugReportObjectTypeEXT type,
									 const char *str) {
	if (!device.get_extensions_func_pointers().debug_marker().enabled)
		return;

	VkDebugMarkerObjectNameInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	info.pNext = nullptr;
	info.pObjectName = str;
	info.object = reinterpret_cast<std::uint64_t>(t);
	info.objectType = type;

	const vk_result res = device.get_extensions_func_pointers().debug_marker().vkDebugMarkerSetObjectNameEXT(device,
																											 &info);
	if (!res) {
		throw vk_exception(res);
	}
}

}

}
}
