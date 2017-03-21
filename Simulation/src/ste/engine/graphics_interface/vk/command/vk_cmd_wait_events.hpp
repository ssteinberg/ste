//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_event.hpp>
#include <vk_pipeline_barrier.hpp>

#include <vector>
#include <initializer_list>

namespace StE {
namespace GL {

class vk_cmd_wait_events : public vk_command {
private:
	std::vector<VkEvent> events;
	const vk_pipeline_barrier &barrier;
	std::vector<VkMemoryBarrier> memory_barriers;
	std::vector<VkBufferMemoryBarrier> buffer_barriers;
	std::vector<VkImageMemoryBarrier> image_barriers;

public:
	vk_cmd_wait_events(const std::initializer_list<VkEvent> &events,
					   const vk_pipeline_barrier &barrier)
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
	vk_cmd_wait_events(const vk_event& event,
					   const vk_pipeline_barrier &barrier)
		: vk_cmd_wait_events({ static_cast<VkEvent>(event) },  barrier)
	{}
	virtual ~vk_cmd_wait_events() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
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
