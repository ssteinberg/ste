//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_command_buffers.hpp>

namespace StE {
namespace GL {

class vk_command_recorder {
private:
	vk_command_buffer *buffer{ nullptr };

public:
	vk_command_recorder(vk_command_buffer &buffer,
						const VkCommandBufferUsageFlags &flags = 0) : buffer(&buffer) {
		buffer.begin(flags);
	}
	~vk_command_recorder() noexcept {
		end();
	}

	vk_command_recorder(vk_command_recorder &&o) noexcept : buffer(o.buffer) { o.buffer = nullptr; }
	vk_command_recorder &operator=(vk_command_recorder &&o) noexcept {
		buffer = o.buffer;
		o.buffer = nullptr;
		return *this;
	}
	vk_command_recorder(const vk_command_recorder &) = delete;
	vk_command_recorder &operator=(const vk_command_recorder &) = delete;

	void end() {
		if (buffer) {
			buffer->end();
			buffer = nullptr;
		}
	}

	auto& operator<<(const vk_command &cmd) {
		cmd(*buffer);
		return *this;
	}
};

}
}
