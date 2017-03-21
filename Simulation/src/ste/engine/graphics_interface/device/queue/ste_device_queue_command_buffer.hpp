//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_command_buffers.hpp>

#include <vk_command_pool.hpp>
#include <vk_command_recorder.hpp>

namespace StE {
namespace GL {

template <VkCommandBufferUsageFlags useage_flags>
class ste_device_queue_command_buffer {
	friend class ste_device_queue;

private:
	vk_command_buffers buffers;

	explicit operator vk_command_buffer() const { return buffers[0]; }

public:
	ste_device_queue_command_buffer(const vk_command_pool &pool,
									const vk_command_buffer_type &type)
		: buffers(pool.allocate_buffers(1, type))
	{}

	ste_device_queue_command_buffer(ste_device_queue_command_buffer&&) = default;
	ste_device_queue_command_buffer &operator=(ste_device_queue_command_buffer&&) = default;

	auto record() {
		return vk_command_recorder(buffers[0], useage_flags);
	}
};

}
}
