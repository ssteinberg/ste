//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_logical_device.hpp>

#include <stdexcept>

namespace ste {
namespace gl {

namespace vk {

template <typename T>
void vk_set_object_name(const vk_logical_device &device,
						const T &t,
						VkDebugReportObjectTypeEXT type,
						const char *str) {
	VkDebugMarkerObjectNameInfoEXT info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	info.pNext = nullptr;
	info.pObjectName = str;
	info.object = t;
	info.objectType = type;

	vk_result res = vkDebugMarkerSetObjectNameEXT(device,
												  &info);
	if (!res) {
		throw vk_exception(res);
	}
}

}

}
}
