//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command_buffers.hpp>
#include <command.hpp>

#include <vector>

namespace StE {
namespace GL {

class cmd_execute_commands : public command {
private:
	std::vector<VkCommandBuffer> buffers;

public:
	cmd_execute_commands(const std::vector<command_buffer> &buffers) {
		this->buffers.reserve(buffers.size());
		for (auto &e : buffers)
			this->buffers.push_back(e);
	}
	virtual ~cmd_execute_commands() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdExecuteCommands(command_buffer, buffers.size(), buffers.data());
	}
};

}
}
