//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout_binding.hpp>

#include <optional.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_descriptor_set_layout {
private:
	optional<VkDescriptorSetLayout> layout;
	const vk_logical_device &device;

public:
	vk_descriptor_set_layout(const vk_logical_device &device,
							 const std::vector<vk_descriptor_set_layout_binding> &bindings) : device(device) {
		std::vector<VkDescriptorSetLayoutBinding> binding_descriptors;
		binding_descriptors.resize(bindings.size());
		for (int i = 0; i < bindings.size(); ++i)
			binding_descriptors[i] = *(bindings.begin() + i);

		VkDescriptorSetLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.bindingCount = binding_descriptors.size();
		create_info.pBindings = binding_descriptors.data();

		VkDescriptorSetLayout layout;
		vk_result res = vkCreateDescriptorSetLayout(device, &create_info, nullptr, &layout);
		if (!res) {
			throw vk_exception(res);
		}

		this->layout = layout;
	}
	~vk_descriptor_set_layout() noexcept {
		destroy_descriptor_set_layout();
	}

	vk_descriptor_set_layout(vk_descriptor_set_layout &&) = default;
	vk_descriptor_set_layout &operator=(vk_descriptor_set_layout &&) = default;
	vk_descriptor_set_layout(const vk_descriptor_set_layout &) = delete;
	vk_descriptor_set_layout &operator=(const vk_descriptor_set_layout &) = delete;

	void destroy_descriptor_set_layout() {
		if (layout) {
			vkDestroyDescriptorSetLayout(device, *this, nullptr);
			layout = none;
		}
	}

	auto& get_descriptor_set_layout() const { return layout.get(); }

	operator VkDescriptorSetLayout() const { return get_descriptor_set_layout(); }
};

}
}
