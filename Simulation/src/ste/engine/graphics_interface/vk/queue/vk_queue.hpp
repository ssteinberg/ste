//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_handle.hpp>
#include <vk_ext_debug_marker.hpp>

#include <vk_logical_device.hpp>
#include <ste_queue_family.hpp>
#include <vk_command_buffers.hpp>
#include <vk_semaphore.hpp>
#include <vk_fence.hpp>

#include <optional.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_queue : public allow_type_decay<vk_queue<host_allocator>, VkQueue> {
public:
	using wait_semaphore_t = std::pair<VkSemaphore, VkPipelineStageFlags>;

private:
	optional<VkQueue> queue;
	ste_queue_family queue_family;

public:
	vk_queue(const vk_logical_device<host_allocator> &device,
			 const ste_queue_family &queue_family,
			 std::uint32_t queue_index,
			 const char *name)
		: queue_family(queue_family) {
		VkQueue q;
		vkGetDeviceQueue(device,
						 static_cast<std::uint32_t>(queue_family),
						 queue_index,
						 &q);

		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										q,
										VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
										name);

		this->queue = q;
	}

	~vk_queue() noexcept {}

	vk_queue(vk_queue &&) = default;
	vk_queue &operator=(vk_queue &&) = default;
	vk_queue(const vk_queue &) = delete;
	vk_queue &operator=(const vk_queue &) = delete;

	/**
	*	@brief	Blocks the caller till the queue has finished processing all the work on the device
	*/
	void wait_idle() const {
		vkQueueWaitIdle(*this);
	}

	/**
	*	@brief	Submits one or more command buffers for execution on the queue
	*
	*	@param	command_buffers		Command buffers to submit
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution, and corresponsing pipeline
	*								stages at which the wait occurs
	*	@param	signal_semaphores	Sempahores to signal once the commands have completed execution
	*	@param	fence				Optional fence, to be signaled when the commands have completed execution
	*/
	void submit(const lib::vector<vk_command_buffer> &command_buffers,
				const lib::vector<wait_semaphore_t> &wait_semaphores,
				const lib::vector<VkSemaphore> &signal_semaphores,
				const vk_fence<host_allocator> *fence = nullptr) const {
		lib::vector<VkCommandBuffer> cb;
		lib::vector<VkSemaphore> wait;
		lib::vector<VkPipelineStageFlags> stages;
		lib::vector<VkSemaphore> signal;

		cb.resize(command_buffers.size());
		for (std::size_t i = 0; i < command_buffers.size(); ++i)
			cb[i] = command_buffers[i];

		wait.resize(wait_semaphores.size());
		stages.resize(wait_semaphores.size());
		for (std::size_t i = 0; i < wait_semaphores.size(); ++i) {
			wait[i] = wait_semaphores[i].first;
			stages[i] = wait_semaphores[i].second;
		}

		signal.resize(signal_semaphores.size());
		for (std::size_t i = 0; i < signal_semaphores.size(); ++i)
			signal[i] = signal_semaphores[i];

		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = static_cast<std::uint32_t>(wait.size());
		info.pWaitSemaphores = wait.data();
		info.pWaitDstStageMask = stages.data();
		info.commandBufferCount = static_cast<std::uint32_t>(cb.size());
		info.pCommandBuffers = cb.data();
		info.signalSemaphoreCount = static_cast<std::uint32_t>(signal.size());
		info.pSignalSemaphores = signal.data();

		const vk_result res = vkQueueSubmit(*this,
											1,
											&info,
											fence != nullptr ? static_cast<VkFence>(*fence) : vk_null_handle);
		if (!res) {
			throw vk_exception(res);
		}
	}

	/**
	*	@brief	Submits a command buffer for execution on the queue
	*
	*	@param	command_buffer		Command buffer to submit
	*	@param	fence				Optional fence, to be signaled when the commands have completed execution
	*/
	void submit(const vk_command_buffer &command_buffer,
				const vk_fence<host_allocator> *fence = nullptr) const {
		return submit({ command_buffer }, {}, {}, fence);
	}

	/**
	*	@brief	Queues bind sparse memory commands on the queue
	*
	*	@param	buffer_binds		Sparse buffer memory (un)bindings opeartions to perform
	*	@param	image_binds			Sparse image memory (un)bindings opeartions to perform
	*	@param	image_opaque_binds	Sparse opaque image memory (un)bindings opeartions to perform
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	@param	fence				Optional fence, to be signaled when the command has completed execution
	*/
	void submit_bind_sparse(const lib::vector<VkSparseBufferMemoryBindInfo> &buffer_binds,
							const lib::vector<VkSparseImageMemoryBindInfo> &image_binds,
							const lib::vector<VkSparseImageOpaqueMemoryBindInfo> &image_opaque_binds,
							const lib::vector<VkSemaphore> &wait_semaphores,
							const lib::vector<VkSemaphore> &signal_semaphores,
							const vk_fence<host_allocator> *fence = nullptr) const {
		lib::vector<VkSemaphore> wait;
		lib::vector<VkSemaphore> signal;

		if (wait_semaphores.size()) {
			wait.reserve(wait_semaphores.size());
			for (auto &e : wait_semaphores)
				wait.push_back(e);
		}

		if (signal_semaphores.size()) {
			signal.reserve(signal_semaphores.size());
			for (auto &e : signal_semaphores)
				signal.push_back(e);
		}

		VkBindSparseInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = static_cast<std::uint32_t>(wait.size());
		info.pWaitSemaphores = wait.data();
		info.bufferBindCount = static_cast<std::uint32_t>(buffer_binds.size());
		info.pBufferBinds = buffer_binds.data();
		info.imageOpaqueBindCount = static_cast<std::uint32_t>(image_opaque_binds.size());
		info.pImageOpaqueBinds = image_opaque_binds.data();
		info.imageBindCount = static_cast<std::uint32_t>(image_binds.size());
		info.pImageBinds = image_binds.data();
		info.signalSemaphoreCount = static_cast<std::uint32_t>(signal.size());
		info.pSignalSemaphores = signal.data();

		const vk_result res = vkQueueBindSparse(*this,
												1,
												&info,
												fence != nullptr ? static_cast<VkFence>(*fence) : vk_null_handle);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto &get() const { return queue.get(); }
	auto get_queue_family_index() const { return queue_family; }
};

}

}
}
