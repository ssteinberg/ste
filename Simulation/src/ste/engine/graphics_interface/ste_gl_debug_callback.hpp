//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_instance.hpp>

#include <lib/string.hpp>
#include <lib/flat_set.hpp>
#include <log.hpp>

namespace ste {
namespace gl {

class ste_gl_debug_callback {
public:
	void operator()(VkDebugReportFlagsEXT flags,
					VkDebugReportObjectTypeEXT objectType,
					std::uint64_t object,
					std::size_t location,
					std::int32_t messageCode,
					const char* pLayerPrefix,
					const char* pMessage,
					const void *ctx) const {
		static const lib::flat_set<std::int32_t> ignored_message_codes = { 10, 11, 12, 1002554381, 1000457229 };

		const lib::string message = lib::string("Vulkan debug report - ") + pLayerPrefix + ": " + pMessage + "\n";

		if (ignored_message_codes.find(messageCode) != ignored_message_codes.end()) {
			// Ignore message
//			ste_log_warn() << message << text::attributes::i("  (ignored)");
			return;
		}

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
