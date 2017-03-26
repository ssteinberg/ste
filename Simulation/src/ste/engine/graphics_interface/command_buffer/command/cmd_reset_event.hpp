//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_event.hpp>

namespace StE {
namespace GL {

class cmd_reset_event : public command {
private:
	const vk_event &event;
	VkPipelineStageFlags stage;

public:
	cmd_reset_event(const vk_event &event,
					   const VkPipelineStageFlags &stage) : event(event), stage(stage) {}
	virtual ~cmd_reset_event() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdResetEvent(command_buffer, event, stage);
	}
};

}
}
