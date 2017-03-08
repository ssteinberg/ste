//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_pipeline_barrier.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_pipeline_barrier : public vk_command {
private:
	const vk_pipeline_barrier &barrier;
	std::vector<VkMemoryBarrier> memory_barriers;
	std::vector<VkBufferMemoryBarrier> buffer_barriers;
	std::vector<VkImageMemoryBarrier> image_barriers;

public:
	vk_cmd_pipeline_barrier(const vk_pipeline_barrier &barrier) : barrier(barrier) {
		const auto &global = barrier.get_global_memory_barriers();
		const auto &buffer = barrier.get_buffer_barriers();
		const auto &image = barrier.get_image_barriers();

		memory_barriers.reserve(global.size());
		for (auto &e : global)
			memory_barriers.push_back(e);

		buffer_barriers.reserve(buffer.size());
		for (auto &e : buffer)
			buffer_barriers.push_back(e);

		image_barriers.reserve(image.size());
		for (auto &e : image)
			image_barriers.push_back(e);
	}
	virtual ~vk_cmd_pipeline_barrier() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdPipelineBarrier(command_buffer,
							 barrier.get_src_stage(),
							 barrier.get_dst_stage(),
							 0,
							 memory_barriers.size(), memory_barriers.data(),
							 buffer_barriers.size(), buffer_barriers.data(),
							 image_barriers.size(), image_barriers.data());
	}
};

}
}
