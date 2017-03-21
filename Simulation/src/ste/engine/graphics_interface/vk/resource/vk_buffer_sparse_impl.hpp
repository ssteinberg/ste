//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_buffer_base.hpp>
#include <vk_queue.hpp>
#include <vk_fence.hpp>

#include <vk_sparse_memory_bind.hpp>

namespace StE {
namespace GL {

template <typename T>
class vk_buffer_sparse_impl : public vk_buffer_base {
	using Base = vk_buffer_base;

public:
	vk_buffer_sparse_impl(const vk_logical_device &device,
						  std::uint64_t count,
						  const VkBufferUsageFlags &usage)
		: Base(device, count * sizeof(T), sizeof(T), usage, true)
	{}
	~vk_buffer_sparse_impl() noexcept {}

	vk_buffer_sparse_impl(vk_buffer_sparse_impl &&) = default;
	vk_buffer_sparse_impl& operator=(vk_buffer_sparse_impl &&) = default;
	vk_buffer_sparse_impl(const vk_buffer_sparse_impl &) = delete;
	vk_buffer_sparse_impl& operator=(const vk_buffer_sparse_impl &) = delete;

	/**
	*	@brief	Queues a bind sparse memory command on the queue
	*
	*	@param	queue				Queue to use
	*	@param	memory_binds		Sparse memory (un)bindings opeartions to perform
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
	*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
	*	@param	fence				Optional fence, to be signaled when the command has completed execution
	*/
	void cmd_bind_sparse_memory(const vk_queue &queue,
								const std::vector<vk_sparse_memory_bind> &memory_binds,
								const std::vector<VkSemaphore> &wait_semaphores,
								const std::vector<VkSemaphore> &signal_semaphores,
								const vk_fence *fence = nullptr) {
		std::vector<VkSemaphore> wait;
		std::vector<VkSemaphore> signal;
		std::vector<VkSparseMemoryBind> binds;

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

		binds.reserve(memory_binds.size());
		for (auto &e : memory_binds) {
			VkSparseMemoryBind b = {};

			b.resourceOffset = e.resource_offset_bytes;
			b.size = e.size_bytes;

			if (e.allocation != nullptr) {
				// Bind
				assert(*e.allocation);
				b.memory = *e.allocation->get_memory();
				b.memoryOffset = (**e.allocation).get_offset();
			}
			else {
				// Unbind
				b.memory = VK_NULL_HANDLE;
			}

			binds.push_back(b);
		}

		VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
		buffer_memory_bind_info.buffer = *this;
		buffer_memory_bind_info.bindCount = binds.size();
		buffer_memory_bind_info.pBinds = binds.data();

		VkBindSparseInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = wait.size();
		info.pWaitSemaphores = wait.data();
		info.bufferBindCount = 1;
		info.pBufferBinds = &buffer_memory_bind_info;
		info.imageOpaqueBindCount = 0;
		info.pImageOpaqueBinds = nullptr;
		info.imageBindCount = 0;
		info.pImageBinds = nullptr;
		info.signalSemaphoreCount = signal.size();
		info.pSignalSemaphores = signal.data();

		vk_result res = vkQueueBindSparse(queue,
										  1, &info,
										  fence != nullptr ? *fence : VK_NULL_HANDLE);
		if (!res) {
			throw vk_exception(res);
		}
	}

	VkMemoryRequirements get_memory_requirements() const {
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(Base::device, *this, &req);

		return req;
	}
};

}
}
