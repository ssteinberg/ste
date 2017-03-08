//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_event.hpp>

namespace StE {
namespace GL {

class vk_cmd_set_event : public vk_command {
private:
	const vk_event &event;
	VkPipelineStageFlags stage;

public:
	vk_cmd_set_event(const vk_event &event,
					 const VkPipelineStageFlags &stage) : event(event), stage(stage) {}
	virtual ~vk_cmd_set_event() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdSetEvent(command_buffer, event, stage);
	}
};

}
}
