//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_handle.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <secondary_command_buffer_inheritance.hpp>

#include <vk_command_buffers.hpp>
#include <vk_command_pool.hpp>
#include <vk_queue.hpp>

#include <host_command.hpp>
#include <command_recorder.hpp>

#include <allow_type_decay.hpp>
#include <vector>
#include "ste_device_queue_secondary_command_buffer.hpp"

namespace ste {
namespace gl {

class ste_device_queue;

class command_buffer : public allow_type_decay<command_buffer, vk::vk_command_buffer> {
	friend command_recorder;
	friend ste_device_queue;

public:
	using commands_t = std::vector<host_command>;

protected:
	vk::vk_command_buffers buffers;
	vk::vk_command_buffer_type type;
	ste_queue_descriptor queue_descriptor;

	mutable commands_t commands;

protected:
	command_buffer(const vk::vk_command_pool &pool,
				   const vk::vk_command_buffer_type &type,
				   const ste_queue_descriptor &queue_descriptor)
		: buffers(pool.allocate_buffers(1, type)),
		type(type),
		queue_descriptor(queue_descriptor)
	{}

	virtual VkCommandBufferUsageFlags record_flags() const = 0;

	auto& get() { return buffers[0]; }
	void push_command(host_command &&command) {
		commands.emplace_back(std::move(command));
	}

	virtual void host_commands_reset_on_submit_if_needed() const = 0;
	void host_commands_reset() const {
		commands.clear();
	}
	void submit_host_commands(const vk::vk_queue &queue) const {
		for (auto &cmd : commands)
			cmd(queue);
		host_commands_reset_on_submit_if_needed();
	}

	template <typename... Args>
	static auto create_recorder(Args&&... args) {
		return command_recorder(std::forward<Args>(args)...);
	}

public:
	virtual ~command_buffer() noexcept {}

	command_buffer(command_buffer&&) = default;
	command_buffer &operator=(command_buffer&& o) = default;

	/**
	*	@brief	Creates a recorder which records the command buffer
	*	
	*	@return	A recorder object. Recording is finished once the object runs out of scope, or can be ended explicitly by calling 
	*			the recorder's finish() method.
	*/
	auto record() {
		return create_recorder(*this, queue_descriptor);
	}

	auto &get_queue_descriptor() const { return queue_descriptor; }
	auto get_type() const { return type; }
	auto& get() const { return buffers[0]; }

private:
	/**
	*	@brief	Starts recording the command buffer
	*/
	virtual void begin() {
		host_commands_reset();
		get().begin(record_flags(), nullptr);
	}
	/**
	*	@brief	Ends recording the command buffer, moving it to pending state. A pending command buffer is consumed 
	*			by submitting it to a device queue.
	*/
	void end() {
		get().end();
	}
};

namespace _internal {

template <
	VkCommandBufferUsageFlags flags, 
	vk::vk_command_buffer_type buffer_type,
	bool resetable,
	bool oneshot
>
class command_buffer_impl : public command_buffer {
	template <VkCommandPoolCreateFlags, bool>
	friend class command_pool;
	friend ste_device_queue;

	using Base = command_buffer;

	static constexpr vk::vk_command_buffer_type type = buffer_type;

protected:
	command_buffer_impl(const vk::vk_command_pool &pool,
						const ste_queue_descriptor &queue_descriptor)
		: Base(pool,
			   type,
			   queue_descriptor)
	{}

	VkCommandBufferUsageFlags record_flags() const override final {
		return flags;
	}

private:
	void host_commands_reset_on_submit_if_needed() const override final {
		// If this is a one-shot buffer, clear out commands
		if (oneshot)
			host_commands_reset();
	}

public:
	command_buffer_impl(command_buffer_impl&&) = default;
	command_buffer_impl &operator=(command_buffer_impl&& o) = default;

	/**
	*	@brief	Resets the command buffer
	*/
	template <
		bool b = resetable,
		typename = std::enable_if_t<b>
	>
	void reset() {
		host_commands_reset();
		get().reset();
	}
	/**
	*	@brief	Resets the command buffer and releases all the resources allocated by the command buffer back to the system.
	*/
	template <
		bool b = resetable,
		typename = std::enable_if_t<b>
	>
	void reset_release() {
		host_commands_reset();
		get().reset_release();
	}
};

template <VkCommandBufferUsageFlags flags, bool resetable, bool oneshot>
class command_buffer_secondary_impl : public _internal::command_buffer_impl<flags, vk::vk_command_buffer_type::secondary, resetable, oneshot> {
	template <VkCommandPoolCreateFlags, bool>
	friend class command_pool;
	friend class command_recorder;
	using Base = _internal::command_buffer_impl<flags, vk::vk_command_buffer_type::secondary, resetable, oneshot>;

private:
	using Base::Base;

	using Base::record;

public:
	command_buffer_secondary_impl(command_buffer_secondary_impl&&) = default;
	command_buffer_secondary_impl &operator=(command_buffer_secondary_impl&& o) = default;

	/**
	*	@brief	Creates a recorder which records the command buffer
	*
	*	@return	A recorder object. Recording is finished once the object runs out of scope, or can be ended explicitly by calling
	*			the recorder's finish() method.
	*/
	auto record(const secondary_command_buffer_inheritance &inheritance) {
		return Base::create_recorder(*this,
									 Base::queue_descriptor,
									 inheritance);
	}

private:
	/**
	*	@brief	Starts recording the secondary command buffer
	*	
	*	@param	inheritance	Command buffer states to inherit from a primary command buffer.
	*/
	virtual void begin(const secondary_command_buffer_inheritance &inheritance) {
		VkCommandBufferInheritanceInfo inheritance_info = {};
		inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritance_info.pNext = nullptr;
		inheritance_info.framebuffer = inheritance.framebuffer ? 
			static_cast<VkFramebuffer>(*inheritance.framebuffer) : 
			vk::vk_null_handle;
		inheritance_info.renderPass = inheritance.render_pass ? 
			static_cast<VkRenderPass>(*inheritance.render_pass) : 
			vk::vk_null_handle;
		inheritance_info.subpass = inheritance.subpass;
		inheritance_info.occlusionQueryEnable = inheritance.occlusion_query_enable;
		inheritance_info.queryFlags = inheritance.query_flags;
		inheritance_info.pipelineStatistics = inheritance.pipeline_statistics;

		Base::host_commands_reset();
		Base::get().begin(Base::record_flags(), &inheritance_info);
	}
};

}

template <bool resetable>
using command_buffer_primary = _internal::command_buffer_impl<
	VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 
	vk::vk_command_buffer_type::primary,
	resetable,
	true
>;
template <bool resetable>
using command_buffer_primary_multishot = _internal::command_buffer_impl<
	VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, 
	vk::vk_command_buffer_type::primary,
	resetable,
	false
>;
template <bool resetable>
using command_buffer_secondary = _internal::command_buffer_secondary_impl<
	VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	resetable,
	false
>;

}
}
