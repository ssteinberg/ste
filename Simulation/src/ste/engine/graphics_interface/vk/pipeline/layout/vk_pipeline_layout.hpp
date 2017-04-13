//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout.hpp>
#include <vk_push_constant_layout.hpp>

#include <optional.hpp>

#include <vector>
#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

class vk_pipeline_layout : public allow_type_decay<vk_pipeline_layout, VkPipelineLayout> {
private:
	optional<VkPipelineLayout> layout;
	std::reference_wrapper<const vk_logical_device> device;

public:
	vk_pipeline_layout(const vk_logical_device &device, 
					   const std::vector<VkDescriptorSetLayout> &set_layouts,
					   const std::vector<vk_push_constant_layout> &push_constant_layouts = {}) : device(device) {
		std::vector<VkPushConstantRange> push_constant_layout_descriptors;
		push_constant_layout_descriptors.resize(push_constant_layouts.size());
		for (std::size_t i = 0; i < push_constant_layouts.size(); ++i)
			push_constant_layout_descriptors[i] = *(push_constant_layouts.begin() + i);

		VkPipelineLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.pushConstantRangeCount = push_constant_layout_descriptors.size();
		create_info.pPushConstantRanges = push_constant_layout_descriptors.data();
		create_info.setLayoutCount = set_layouts.size();
		create_info.pSetLayouts = set_layouts.data();

		VkPipelineLayout pipeline_layout;
		vk_result res = vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout);
		if (!res) {
			throw vk_exception(res);
		}

		this->layout = pipeline_layout;
	}
	~vk_pipeline_layout() noexcept {
		destroy_pipeline_layout();
	}

	vk_pipeline_layout(vk_pipeline_layout &&) = default;
	vk_pipeline_layout &operator=(vk_pipeline_layout &&o) = default;
	vk_pipeline_layout(const vk_pipeline_layout &) = delete;
	vk_pipeline_layout &operator=(const vk_pipeline_layout &) = delete;

	void destroy_pipeline_layout() {
		if (layout) {
			vkDestroyPipelineLayout(device.get(), *this, nullptr);
			layout = none;
		}
	}

	auto& get() const { return layout.get(); }
};

}
}
