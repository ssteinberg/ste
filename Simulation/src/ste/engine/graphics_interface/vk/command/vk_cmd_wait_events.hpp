//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_event.hpp>

#include <vector>
#include <initializer_list>

namespace StE {
namespace GL {

class vk_cmd_wait_events : public vk_command {
private:
	std::vector<VkEvent> events;
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

public:
	vk_cmd_wait_events(const std::initializer_list<const vk_event&> &events,
					   VkPipelineStageFlags src_stage,
					   VkPipelineStageFlags dst_stage)
		: src_stage(src_stage), dst_stage(dst_stage) 
	{
		this->events.reserve(events.size());
		for (auto &e : events)
			this->events.push_back(e);
	}
	virtual ~vk_cmd_wait_events() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		//! TODO
		vkCmdWaitEvents(command_buffer, events.size(), events.data(),
						src_stage, dst_stage,
						0, nullptr,
						0, nullptr,
						0, nullptr);
	}
};

}
}
