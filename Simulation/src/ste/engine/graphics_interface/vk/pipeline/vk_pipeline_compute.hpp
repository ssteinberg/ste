//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_shader.hpp>
#include <vk_pipeline_layout.hpp>
#include <vk_pipeline_cache.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_pipeline_compute {
private:
	optional<VkPipeline> pipeline;
	const vk_logical_device &device;

public:
	vk_pipeline_compute(const vk_logical_device &device,
						const vk_shader &shader_module,
						const vk_pipeline_layout &layout,
						const optional<vk_pipeline_cache> &cache = none) : device(device) {
		auto stage_info = shader_module.shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT);

		VkComputePipelineCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.stage = stage_info;
		create_info.layout = layout;
		create_info.basePipelineHandle = VK_NULL_HANDLE;
		create_info.basePipelineIndex = 0;

		VkPipeline pipeline;
		vk_result res = vkCreateComputePipelines(device, 
												 cache ? cache.get().get_pipeline_cache() : VK_NULL_HANDLE, 
												 1, 
												 &create_info, 
												 nullptr, 
												 &pipeline);
		if (!res) {
			throw vk_exception(res);
		}

		this->pipeline = pipeline;
	}
	~vk_pipeline_compute() noexcept {
		destroy_pipeline();
	}

	vk_pipeline_compute(vk_pipeline_compute &&) = default;
	vk_pipeline_compute &operator=(vk_pipeline_compute &&) = default;
	vk_pipeline_compute(const vk_pipeline_compute &) = delete;
	vk_pipeline_compute &operator=(const vk_pipeline_compute &) = delete;

	void destroy_pipeline() {
		if (pipeline) {
			vkDestroyPipeline(device, *this, nullptr);
			pipeline = none;
		}
	}

	auto& get_pipeline() const { return pipeline.get(); }

	operator VkPipeline() const { return get_pipeline(); }
};

}
}
