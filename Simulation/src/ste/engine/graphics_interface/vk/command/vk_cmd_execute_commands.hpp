//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command_buffers.hpp>
#include <vk_command.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_execute_commands : public vk_command {
private:
	std::vector<VkCommandBuffer> buffers;

public:
	vk_cmd_execute_commands(const std::vector<vk_command_buffer> &buffers) {
		this->buffers.reserve(buffers.size());
		for (auto &e : buffers)
			this->buffers.push_back(e);
	}
	virtual ~vk_cmd_execute_commands() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdExecuteCommands(command_buffer, buffers.size(), buffers.data());
	}
};

}
}
