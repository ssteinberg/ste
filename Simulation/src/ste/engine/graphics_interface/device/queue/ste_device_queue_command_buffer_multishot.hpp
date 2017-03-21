//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_command_buffers.hpp>

#include <vk_command_pool.hpp>
#include <vk_command_recorder.hpp>

#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class ste_device_queue_command_buffer_multishot : public allow_class_decay<ste_device_queue_command_buffer_multishot, vk_command_buffer> {
	friend class ste_device_queue;

private:
	vk_command_buffers buffers;

public:
	ste_device_queue_command_buffer_multishot(const vk_command_pool &pool,
											  const vk_command_buffer_type &type)
		: buffers(pool.allocate_buffers(1, type))
	{}

	ste_device_queue_command_buffer_multishot(ste_device_queue_command_buffer_multishot&&) = default;
	ste_device_queue_command_buffer_multishot &operator=(ste_device_queue_command_buffer_multishot&&) = default;

	auto record(const VkCommandBufferUsageFlags &flags = 0) {
		return vk_command_recorder(buffers[0], flags);
	}

	auto& get() const { return buffers[0]; }
};

}
}
