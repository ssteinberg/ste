//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set_layout.hpp>
#include <vk_push_constant_layout.hpp>

#include <optional.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_pipeline_layout {
private:
	optional<VkPipelineLayout> layout;
	const vk_logical_device &device;

public:
	vk_pipeline_layout(const vk_logical_device &device, 
					   const std::vector<vk_descriptor_set_layout> &set_layouts,
					   const std::vector<vk_push_constant_layout> &push_constants_layout = {}) : device(device) {
		std::vector<VkDescriptorSetLayout> set_layout_descriptors;
		set_layout_descriptors.resize(set_layouts.size());
		for (int i = 0; i < set_layouts.size(); ++i)
			set_layout_descriptors[i] = *(set_layouts.begin() + i);

		std::vector<VkPushConstantRange> push_constant_layout_descriptors;
		push_constant_layout_descriptors.resize(push_constants_layout.size());
		for (int i = 0; i < push_constants_layout.size(); ++i)
			push_constant_layout_descriptors[i] = *(push_constants_layout.begin() + i);

		VkPipelineLayoutCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.pushConstantRangeCount = push_constant_layout_descriptors.size();
		create_info.pPushConstantRanges = push_constant_layout_descriptors.data();
		create_info.setLayoutCount = set_layout_descriptors.size();
		create_info.pSetLayouts = set_layout_descriptors.data();

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
	vk_pipeline_layout &operator=(vk_pipeline_layout &&) = default;
	vk_pipeline_layout(const vk_pipeline_layout &) = delete;
	vk_pipeline_layout &operator=(const vk_pipeline_layout &) = delete;

	void destroy_pipeline_layout() {
		if (layout) {
			vkDestroyPipelineLayout(device, *this, nullptr);
			layout = none;
		}
	}

	auto& get_pipeline_layout() const { return layout.get(); }

	operator VkPipelineLayout() const { return get_pipeline_layout(); }
};

}
}
