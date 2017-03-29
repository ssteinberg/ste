//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_pipeline.hpp>
#include <vk_logical_device.hpp>
#include <vk_shader.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_pipeline_cache.hpp>

namespace StE {
namespace GL {

class vk_pipeline_compute : public vk_pipeline {

public:
	vk_pipeline_compute(const vk_logical_device &device,
						const vk_shader &shader_module,
						const vk_pipeline_layout &layout,
						const vk_pipeline_cache *cache = nullptr) : vk_pipeline(device) {
		vk_shader::shader_stage_info_t stage_info;
		shader_module.shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT, stage_info);

		VkComputePipelineCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.stage = stage_info.stage_info;
		create_info.layout = layout;
		create_info.basePipelineHandle = VK_NULL_HANDLE;
		create_info.basePipelineIndex = 0;

		VkPipeline pipeline;
		vk_result res = vkCreateComputePipelines(device, 
												 cache!=nullptr ? *cache : VK_NULL_HANDLE, 
												 1, 
												 &create_info, 
												 nullptr, 
												 &pipeline);
		if (!res) {
			throw vk_exception(res);
		}

		this->pipeline = pipeline;
	}
	~vk_pipeline_compute() noexcept {}

	vk_pipeline_compute(vk_pipeline_compute &&) = default;
	vk_pipeline_compute &operator=(vk_pipeline_compute &&) = default;
	vk_pipeline_compute(const vk_pipeline_compute &) = delete;
	vk_pipeline_compute &operator=(const vk_pipeline_compute &) = delete;
};

}
}
