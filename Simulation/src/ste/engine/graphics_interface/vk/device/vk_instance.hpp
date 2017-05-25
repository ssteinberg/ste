//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <ste_version.hpp>

#include <vk_physical_device_descriptor.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <lib/vector.hpp>
#include <lib/unique_ptr.hpp>
#include <allow_type_decay.hpp>
#include <optional.hpp>
#include <vk_host_allocator.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_instance : public allow_type_decay<vk_instance<host_allocator>, VkInstance> {
private:
	optional<VkInstance> instance;
	const lib::vector<const char*> instance_extensions;
	const lib::vector<const char*> instance_layers;

public:
	vk_instance(const char *app_name,
				unsigned app_version,
				const lib::vector<const char*> &instance_extensions,
				const lib::vector<const char*> &instance_layers)
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
		inst_info.enabledExtensionCount = static_cast<std::uint32_t>(instance_extensions.size());
		inst_info.ppEnabledExtensionNames = &instance_extensions[0];
		inst_info.enabledLayerCount = static_cast<std::uint32_t>(instance_layers.size());
		inst_info.ppEnabledLayerNames = &instance_layers[0];
		inst_info.pApplicationInfo = &vk_app_info;

		VkInstance inst;
		vk_result res = vkCreateInstance(&inst_info, &host_allocator::allocation_callbacks(), &inst);
		if (!res) {
			throw vk_exception(res);
		}

		instance = inst;
	}

	vk_instance(vk_instance&&) = default;
	vk_instance &operator=(vk_instance&&) = default;
	vk_instance(const vk_instance &) = delete;
	vk_instance &operator=(const vk_instance &) = delete;

	~vk_instance() noexcept {
		destroy_instance();
	}

	auto &get() const { return instance.get(); }

	void destroy_instance() {
		if (instance) {
			vkDestroyInstance(instance.get(), &host_allocator::allocation_callbacks());
		}
		instance = none;
	}

	auto &enabled_layers() const { return instance_layers; }
	auto &enabled_extensions() const { return instance_extensions; }
};

}

}
}
