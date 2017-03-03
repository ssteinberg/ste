//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_command_buffers.hpp>

namespace StE {
namespace GL {

class vk_command_recorder {
private:
	vk_command_buffer &buffer;

public:
	vk_command_recorder(vk_command_buffer &buffer,
						const VkCommandBufferUsageFlags &flags = 0) : buffer(buffer) {
		buffer.begin(flags);
	}
	~vk_command_recorder() noexcept {
		buffer.end();
	}

	vk_command_recorder(vk_command_recorder &&) = default;
	vk_command_recorder &operator=(vk_command_recorder &&) = default;
	vk_command_recorder(const vk_command_recorder &) = delete;
	vk_command_recorder &operator=(const vk_command_recorder &) = delete;

	auto& operator<<(const vk_command &cmd) {
		cmd(buffer);
		return *this;
	}
};

}
}
