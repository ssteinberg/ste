//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_global_memory_barrier.hpp>
#include <vk_buffer_memory_barrier.hpp>
#include <vk_image_memory_barrier.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_pipeline_barrier {
private:
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;
	std::vector<vk_global_memory_barrier> global_memory_barriers;
	std::vector<vk_buffer_memory_barrier> buffer_barriers;
	std::vector<vk_image_memory_barrier>  image_barriers;

public:
	vk_pipeline_barrier(const VkPipelineStageFlags &src_stage,
						const VkPipelineStageFlags &dst_stage,
						const std::vector<vk_global_memory_barrier> &global_memory_barriers = {},
						const std::vector<vk_buffer_memory_barrier> &buffer_barriers = {},
						const std::vector<vk_image_memory_barrier>  &image_barriers = {})
		: src_stage(src_stage), dst_stage(dst_stage),
		global_memory_barriers(global_memory_barriers),
		buffer_barriers(buffer_barriers),
		image_barriers(image_barriers)
	{}
	~vk_pipeline_barrier() noexcept {}

	vk_pipeline_barrier(vk_pipeline_barrier &&) = default;
	vk_pipeline_barrier &operator=(vk_pipeline_barrier &&) = default;
	vk_pipeline_barrier(const vk_pipeline_barrier &) = default;
	vk_pipeline_barrier &operator=(const vk_pipeline_barrier &) = default;

	auto& get_src_stage() const { return src_stage; }
	auto& get_dst_stage() const { return dst_stage; }
	auto& get_global_memory_barriers() const { return global_memory_barriers; }
	auto& get_buffer_barriers() const { return buffer_barriers; }
	auto& get_image_barriers() const { return image_barriers; }
};

}
}
