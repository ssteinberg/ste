//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_event.hpp>
#include <pipeline_stage.hpp>

namespace ste {
namespace gl {

class cmd_set_event : public command {
private:
	VkEvent event;
	pipeline_stage stage;

public:
	cmd_set_event(cmd_set_event&&) = default;
	cmd_set_event(const cmd_set_event&) = default;
	cmd_set_event &operator=(cmd_set_event&&) = default;
	cmd_set_event &operator=(const cmd_set_event&) = default;

	cmd_set_event(const vk::vk_event<> &event,
				  const pipeline_stage &stage) : event(event), stage(stage) {}
	virtual ~cmd_set_event() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdSetEvent(command_buffer, event, stage);
	}
};

}
}
