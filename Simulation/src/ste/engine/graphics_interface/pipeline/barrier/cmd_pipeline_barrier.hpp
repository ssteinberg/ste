//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <pipeline_barrier.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_pipeline_barrier : public command {
private:
	const pipeline_barrier &barrier;

public:
	cmd_pipeline_barrier(const pipeline_barrier &barrier) : barrier(barrier) {
	}
	virtual ~cmd_pipeline_barrier() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &recorder) const override final {
		const auto &global = barrier.get_global_memory_barriers();
		const auto &buffer = barrier.get_buffer_barriers();
		const auto &image = barrier.get_image_barriers();

		lib::vector<VkMemoryBarrier> memory_barriers;
		lib::vector<VkBufferMemoryBarrier> buffer_barriers;
		lib::vector<VkImageMemoryBarrier> image_barriers;

		memory_barriers.reserve(global.size());
		for (auto &e : global)
			memory_barriers.push_back(e);

		buffer_barriers.reserve(buffer.size());
		for (auto &e : buffer)
			buffer_barriers.push_back(e);

		image_barriers.reserve(image.size());
		for (auto &e : image)
			image_barriers.push_back(e);

		vkCmdPipelineBarrier(command_buffer,
							 static_cast<VkPipelineStageFlags>(barrier.get_src_stage()),
							 static_cast<VkPipelineStageFlags>(barrier.get_dst_stage()),
							 0,
							 static_cast<std::uint32_t>(memory_barriers.size()), 
							 memory_barriers.data(),
							 static_cast<std::uint32_t>(buffer_barriers.size()), 
							 buffer_barriers.data(),
							 static_cast<std::uint32_t>(image_barriers.size()), 
							 image_barriers.data());
	}
};

}
}
