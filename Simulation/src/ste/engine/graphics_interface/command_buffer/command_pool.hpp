//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <vk_command_pool.hpp>

#include <command_buffer.hpp>
#include <ste_resource_pool.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <VkCommandPoolCreateFlags flags, bool resetable_buffer>
class command_pool :
	public allow_type_decay<command_pool<flags, resetable_buffer>, vk::vk_command_pool>,
	public ste_resource_pool_resetable_trait<const vk::vk_logical_device &, ste_queue_descriptor>
{
private:
	vk::vk_command_pool pool;
	const ste_queue_descriptor queue_descriptor;

public:
	command_pool(const vk::vk_logical_device &device,
				 const ste_queue_descriptor &queue_descriptor)
		: pool(device,
			   queue_descriptor.family,
			   flags),
		queue_descriptor(queue_descriptor)
	{}
	~command_pool() noexcept {}

	command_pool(command_pool&&) = default;
	command_pool &operator=(command_pool&&) = default;

	/**
	 *	@brief	Allocates a new primary one-shot command buffer
	 */
	auto allocate_primary_buffer() const {
		return command_buffer_primary<resetable_buffer>(pool,
														queue_descriptor);
	}
	/**
	*	@brief	Allocates a new primary reusable command buffer
	*/
	auto allocate_primary_multishot_buffer() const {
		return command_buffer_primary_multishot<resetable_buffer>(pool,
																  queue_descriptor);
	}
	/**
	*	@brief	Allocates a new secondary resuable command buffer
	*/
	auto allocate_secondary_buffer() const {
		return command_buffer_secondary<resetable_buffer>(pool,
														  queue_descriptor);
	}

	/**
	*	@brief	Resets the pool, resetting all command buffers allocated from this pool back to initial state.
	*/
	void reset() override {
		pool.reset();
	}

	/**
	*	@brief	Resets the pool, releasing all allocated resources to the system, and resets all command buffers
	*			allocated from this pool.
	*/
	void reset_release() {
		pool.reset_release();
	}

	/**
	*	@brief	From the spec:
	*			Trimming a command pool may be useful to return unused memory back to the system, returning the total outstanding memory allocated by the pool back to a more "average" value.
	*			In most cases trimming will result in a reduction in allocated but unused memory, but it does not guarantee the "ideal" behaviour.
	*			Trimming may be an expensive operation, and should not be called frequently. Trimming should be treated as a way to relieve memory pressure after application-known points when
	*			there exists enough unused memory that the cost of trimming is "worth" it.
	*/
	void trim() {
		pool.trim();
	}

	auto &get_queue_descriptor() const { return queue_descriptor; }
	auto &get() const { return pool; }
};

}

using command_pool = _internal::command_pool<0, false>;
using command_pool_transient = _internal::command_pool<VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, false>;
using command_pool_resetable = _internal::command_pool<VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, true>;
using command_pool_transient_resetable = _internal::command_pool<VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, true>;


}
}
