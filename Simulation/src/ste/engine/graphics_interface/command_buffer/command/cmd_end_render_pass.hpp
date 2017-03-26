//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

namespace StE {
namespace GL {

class cmd_end_render_pass : public command {
public:
	cmd_end_render_pass() {}
	virtual ~cmd_end_render_pass() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdEndRenderPass(command_buffer);
	}
};

}
}
