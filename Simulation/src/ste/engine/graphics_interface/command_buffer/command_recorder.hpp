//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <host_command.hpp>
#include <ste_device_queue_descriptors.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A command recorder records commands into a command buffer. Recording a buffer implicitly resets the buffer,
*			any previous recording is lost
*/
class command_recorder {
	friend class command_buffer;
	template <VkCommandBufferUsageFlags flags, bool resetable>
	friend class command_buffer_secondary_impl;

private:
	command_buffer *buffer;
	const ste_queue_descriptor queue_descriptor;

private:
	template <typename buffer_type, typename... Ts>
	command_recorder(buffer_type &buffer,
					 const ste_queue_descriptor &queue_descriptor,
					 Ts&&... ts)
		: buffer(&buffer), 
		queue_descriptor(queue_descriptor) {
		buffer.begin(std::forward<Ts>(ts)...);
	}

public:
	~command_recorder() noexcept {
		end();
	}

	command_recorder(command_recorder &&o) noexcept
		: buffer(o.buffer), 
		queue_descriptor(queue_descriptor)
	{
		o.buffer = nullptr;
	}
	command_recorder(const command_recorder &) = delete;
	command_recorder &operator=(const command_recorder &) = delete;

	void end();

	/**
	*	@brief	Record a command on the command buffer
	*
	*	@param	cmd	Command to record
	*/
	command_recorder& operator<<(const command &cmd);
	/**
	*	@brief	Record a command on the command buffer
	*
	*	@param	cmd	Command to record
	*/
	command_recorder& operator<<(host_command &&cmd);

	auto &get_queue_descriptor() const { return queue_descriptor; }
};

}
}
