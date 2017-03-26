//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <ste_version.hpp>

#include <vk_physical_device_descriptor.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <vector>
#include <memory>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class vk_instance : public allow_type_decay<vk_instance, VkInstance> {
private:
	VkInstance instance{ nullptr };
	const std::vector<const char*> instance_extensions;
	const std::vector<const char*> instance_layers;

public:
	vk_instance(const char *app_name,
				unsigned app_version,
				const std::vector<const char*> &instance_extensions,
				const std::vector<const char*> &instance_layers) 
		: instance_extensions(instance_extensions), instance_layers(instance_layers)
	{
		VkApplicationInfo vk_app_info = {};
		vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vk_app_info.pNext = nullptr;
		vk_app_info.pApplicationName = app_name;
		vk_app_info.applicationVersion = app_version;
		vk_app_info.pEngineName = ste_name;
		vk_app_info.engineVersion = (ste_version_major << 16) + ste_version_minor;
		vk_app_info.apiVersion = vk_api_version;

		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = nullptr;
		inst_info.flags = 0;
		inst_info.enabledExtensionCount = instance_extensions.size();
		inst_info.ppEnabledExtensionNames = &instance_extensions[0];
		inst_info.enabledLayerCount = instance_layers.size();
		inst_info.ppEnabledLayerNames = &instance_layers[0];
		inst_info.pApplicationInfo = &vk_app_info;

		vk_result res = vkCreateInstance(&inst_info, nullptr, &instance);
		if (!res) {
			throw vk_exception(res);
		}
	}

	~vk_instance() {
		if (instance) {
			vkDestroyInstance(instance, nullptr);
		}
		instance = nullptr;
	}

	vk_instance(vk_instance &&m) noexcept
		: instance(m.instance),
		instance_extensions(std::move(m.instance_extensions)),
		instance_layers(std::move(m.instance_layers))
	{
		m.instance = nullptr;
	}

	auto &get() const { return instance; }

	auto &enabled_layers() const { return instance_layers; }
	auto &enabled_extensions() const { return instance_extensions; }
};

}
}
