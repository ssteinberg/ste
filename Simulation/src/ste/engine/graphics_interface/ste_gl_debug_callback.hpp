//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_instance.hpp>

#include <lib/string.hpp>
#include <log.hpp>

namespace ste {
namespace gl {

class ste_gl_debug_callback {
public:
	void operator()(VkDebugReportFlagsEXT flags,
					VkDebugReportObjectTypeEXT objectType,
					uint64_t object,
					size_t location,
					int32_t messageCode,
					const char* pLayerPrefix,
					const char* pMessage,
					const void *ctx) const {
		const lib::string message = lib::string("Vulkan debug report - ") + pLayerPrefix + ": " + pMessage + "\n";
		switch (flags) {
		case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
			ste_log_warn() << message;
			break;
		case VK_DEBUG_REPORT_ERROR_BIT_EXT:
			ste_log_error() << message;
			break;
		case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		default:
			ste_log() << message;
			break;
		}
	}
};

}
}
