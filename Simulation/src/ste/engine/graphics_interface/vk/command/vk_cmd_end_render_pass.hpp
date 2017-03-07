//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>

namespace StE {
namespace GL {

class vk_cmd_end_render_pass : public vk_command {
public:
	vk_cmd_end_render_pass() {}
	virtual ~vk_cmd_end_render_pass() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdEndRenderPass(command_buffer);
	}
};

}
}
