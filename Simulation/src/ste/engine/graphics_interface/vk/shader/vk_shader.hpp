//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <string>

namespace StE {
namespace GL {

class vk_shader {
private:
	VkShaderModule module{ VK_NULL_HANDLE };
	const vk_logical_device &device;

public:
	vk_shader(const vk_logical_device &device, const std::string &code) : device(device) {
		VkShaderModuleCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.codeSize = code.length();
		create_info.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

		VkShaderModule shader_module;
		vk_result res = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
		if (!res) {
			throw vk_exception(res);
		}

		this->module = shader_module;
	}
	~vk_shader() noexcept {
		destroy_shader_module();
	}

	vk_shader(vk_shader &&) = default;
	vk_shader &operator=(vk_shader &&) = default;
	vk_shader(const vk_shader &) = delete;
	vk_shader &operator=(const vk_shader &) = delete;

	void destroy_shader_module() {
		if (module) {
			vkDestroyShaderModule(device, *this, nullptr);
			module = VK_NULL_HANDLE;
		}
	}

	auto& get_shader_module() const { return module; }

	operator VkShaderModule() const { return get_shader_module(); }
};

}
}
