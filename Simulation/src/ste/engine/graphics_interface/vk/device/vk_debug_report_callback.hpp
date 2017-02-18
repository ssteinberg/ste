//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_instance.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_debug_report_callback {
private:
	optional<VkDebugReportCallbackEXT> handle;
	const vk_instance &instance;

public:
	vk_debug_report_callback(const vk_instance &instance,
							 void *user_data,
							 PFN_vkDebugReportCallbackEXT callback) : instance(instance) {
		VkDebugReportCallbackCreateInfoEXT debug_info;
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_info.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
						   VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
						   VK_DEBUG_REPORT_ERROR_BIT_EXT |
						   VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		debug_info.pNext = nullptr;
		debug_info.pUserData = user_data;
		debug_info.pfnCallback = callback;

		auto f = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
		
		if (f) {
			VkDebugReportCallbackEXT debug_report_handle;
			vk_result res = f(instance, &debug_info, nullptr, &debug_report_handle);
			if (!res) {
				throw vk_exception(res);
			}

			this->handle = debug_report_handle;
		}
	}
	~vk_debug_report_callback() noexcept {
		if (handle) {
			auto f = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

			assert(f);
			if (f) {
				auto h = handle.get();
				f(instance, h, nullptr);
			}
		}
		handle = none;
	}

	vk_debug_report_callback(vk_debug_report_callback &&) = default;
	vk_debug_report_callback &operator=(vk_debug_report_callback &&) = default;
	vk_debug_report_callback(const vk_debug_report_callback &) = delete;
	vk_debug_report_callback &operator=(const vk_debug_report_callback &) = delete;
};

}
}
