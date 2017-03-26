//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <pipeline_barrier.hpp>

#include <vector>

namespace StE {
namespace GL {

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

		std::vector<VkMemoryBarrier> memory_barriers;
		std::vector<VkBufferMemoryBarrier> buffer_barriers;
		std::vector<VkImageMemoryBarrier> image_barriers;

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
							 barrier.get_src_stage(),
							 barrier.get_dst_stage(),
							 0,
							 memory_barriers.size(), memory_barriers.data(),
							 buffer_barriers.size(), buffer_barriers.data(),
							 image_barriers.size(), image_barriers.data());

		// Add queue transition and image layout transform host effects
		recorder << host_command([image, buffer](const vk_queue &) {
			for (auto &image_barrier : image) {
				if (image_barrier.image_layout)
					image_barrier.image_layout->image_layout.layout.store(image_barrier.new_layout, std::memory_order_release);
				if (image_barrier.dst_queue_family != VK_QUEUE_FAMILY_IGNORED)
					image_barrier.queue_ownership->queue_ownership.family.store(ste_queue_family(image_barrier.dst_queue_family),
																				std::memory_order_release);
			}
			for (auto &buffer_barrier : buffer) {
				if (buffer_barrier.dst_queue_family != VK_QUEUE_FAMILY_IGNORED)
					buffer_barrier.queue_ownership->queue_ownership.family.store(ste_queue_family(buffer_barrier.dst_queue_family), 
																				 std::memory_order_release);
			}
		});
	}
};

}
}
