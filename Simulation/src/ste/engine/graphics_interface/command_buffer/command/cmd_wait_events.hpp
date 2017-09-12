//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_event.hpp>
#include <pipeline_barrier.hpp>

#include <lib/vector.hpp>
#include <initializer_list>

namespace ste {
namespace gl {

class cmd_wait_events : public command {
private:
	lib::vector<VkEvent> events;
	const pipeline_barrier &barrier;
	lib::vector<VkMemoryBarrier> memory_barriers;
	lib::vector<VkBufferMemoryBarrier> buffer_barriers;
	lib::vector<VkImageMemoryBarrier> image_barriers;

public:
	cmd_wait_events(cmd_wait_events&&) = default;
	cmd_wait_events(const cmd_wait_events&) = default;
	cmd_wait_events &operator=(cmd_wait_events&&) = default;
	cmd_wait_events &operator=(const cmd_wait_events&) = default;

	cmd_wait_events(const std::initializer_list<VkEvent> &events,
					const pipeline_barrier &barrier)
		: barrier(barrier)
	{
		this->events.reserve(events.size());
		for (auto &e : events)
			this->events.push_back(e);

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
	cmd_wait_events(const vk::vk_event<>& event,
					const pipeline_barrier &barrier)
		: cmd_wait_events({ static_cast<VkEvent>(event) }, barrier)
	{}
	virtual ~cmd_wait_events() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdWaitEvents(command_buffer, 
						static_cast<std::uint32_t>(events.size()), events.data(),
						static_cast<VkPipelineStageFlags>(barrier.get_src_stage()),
						static_cast<VkPipelineStageFlags>(barrier.get_dst_stage()),
						static_cast<std::uint32_t>(memory_barriers.size()), memory_barriers.data(),
						static_cast<std::uint32_t>(buffer_barriers.size()), buffer_barriers.data(),
						static_cast<std::uint32_t>(image_barriers.size()),  image_barriers.data());
	}
};

}
}
