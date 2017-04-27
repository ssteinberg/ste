//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_event.hpp>
#include <pipeline_barrier.hpp>

#include <vector>
#include <initializer_list>

namespace ste {
namespace gl {

class cmd_wait_events : public command {
private:
	std::vector<VkEvent> events;
	const pipeline_barrier &barrier;
	std::vector<VkMemoryBarrier> memory_barriers;
	std::vector<VkBufferMemoryBarrier> buffer_barriers;
	std::vector<VkImageMemoryBarrier> image_barriers;

public:
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
	cmd_wait_events(const vk::vk_event& event,
					const pipeline_barrier &barrier)
		: cmd_wait_events({ static_cast<VkEvent>(event) }, barrier)
	{}
	virtual ~cmd_wait_events() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdWaitEvents(command_buffer, events.size(), events.data(),
						barrier.get_src_stage(),
						barrier.get_dst_stage(),
						memory_barriers.size(), memory_barriers.data(),
						buffer_barriers.size(), buffer_barriers.data(),
						image_barriers.size(), image_barriers.data());
	}
};

}
}
