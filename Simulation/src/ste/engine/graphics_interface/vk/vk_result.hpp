//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <lib/string.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_result {
private:
	VkResult code{ VK_SUCCESS };

public:
	vk_result() = default;
	vk_result(const VkResult &res) : code(res) {}

	vk_result(vk_result&&) = default;
	vk_result(const vk_result&) = default;
	vk_result &operator=(vk_result&&) = default;
	vk_result &operator=(const vk_result&) = default;

	vk_result &operator=(const VkResult &res) {
		code = res;
		return *this;
	}

	auto get() const { return code; }
	lib::string string() const {
		switch (code) {
		case VK_SUCCESS: return "Command successfully completed";
		case VK_NOT_READY: return "A fence or query has not yet completed";
		case VK_TIMEOUT: return "A wait operation has not completed in the specified time";
		case VK_EVENT_SET: return "An event is signaled";
		case VK_EVENT_RESET: return "An event is unsignaled";
		case VK_INCOMPLETE: return "A return array was too small for the result";
		case VK_ERROR_OUT_OF_HOST_MEMORY: return "A host memory allocation has failed";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "A device memory allocation has failed";
		case VK_ERROR_INITIALIZATION_FAILED: return "Initialization of an object could not be completed for implementation-specific reasons";
		case VK_ERROR_DEVICE_LOST: return "The logical or physical device has been lost";
		case VK_ERROR_MEMORY_MAP_FAILED: return "Mapping of a memory object has failed";
		case VK_ERROR_LAYER_NOT_PRESENT: return "A requested layer is not present or could not be loaded";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "A requested extension is not supported";
		case VK_ERROR_FEATURE_NOT_PRESENT: return "A requested feature is not supported";
		case VK_ERROR_INCOMPATIBLE_DRIVER: return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
		case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects of the type have already been created";
		case VK_ERROR_FORMAT_NOT_SUPPORTED: return "A requested format is not supported on this device";
		case VK_ERROR_FRAGMENTED_POOL: return "A requested pool allocation has failed due to fragmentation of the pool’s memory";
		default: return "Unknown code";
		}
	}

	bool operator==(const VkResult &res) const { return code == res; }
	bool operator==(const vk_result &res) const { return code == res.code; }

	auto operator!() const { return code != VK_SUCCESS; }
	operator bool() const { return code == VK_SUCCESS; }
	operator VkResult() const { return get(); }
	explicit operator lib::string() const { return string(); }
};

}

}
}
