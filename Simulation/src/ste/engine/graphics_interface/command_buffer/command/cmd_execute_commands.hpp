//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command_buffer.hpp>
#include <command.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_execute_commands : public command {
private:
	lib::vector<VkCommandBuffer> buffers;

public:
	cmd_execute_commands(const command_buffer &buffer) {
		buffers.push_back(buffer);
	}
	cmd_execute_commands(const lib::vector<std::reference_wrapper<command_buffer>> &buffers) {
		this->buffers.reserve(buffers.size());
		for (auto &e : buffers)
			this->buffers.push_back(e.get());
	}
	virtual ~cmd_execute_commands() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdExecuteCommands(command_buffer, buffers.size(), buffers.data());
	}
};

}
}
