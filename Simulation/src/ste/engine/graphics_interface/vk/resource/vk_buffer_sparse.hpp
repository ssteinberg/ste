//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_handle.hpp>

#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_buffer.hpp>
#include <vk_queue.hpp>
#include <vk_fence.hpp>

#include <vk_sparse_memory_bind.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_buffer_sparse : public vk_buffer {
	using Base = vk_buffer;

private:
	std::uint32_t element_size;
	std::uint64_t count;

public:
	vk_buffer_sparse(const vk_logical_device &device,
					 std::uint32_t element_size,
					 std::uint64_t count,
					 const VkBufferUsageFlags &usage)
		: Base(device, count * element_size, usage, true),
		element_size(element_size),
		count(count)
	{}
	~vk_buffer_sparse() noexcept {}

	vk_buffer_sparse(vk_buffer_sparse &&) = default;
	vk_buffer_sparse& operator=(vk_buffer_sparse &&) = default;
	vk_buffer_sparse(const vk_buffer_sparse &) = delete;
	vk_buffer_sparse& operator=(const vk_buffer_sparse &) = delete;

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
				b.memory = vk::vk_null_handle;
			}

			binds.push_back(b);
		}

		VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
		buffer_memory_bind_info.buffer = *this;
		buffer_memory_bind_info.bindCount = static_cast<std::uint32_t>(binds.size());
		buffer_memory_bind_info.pBinds = binds.data();

		VkBindSparseInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = static_cast<std::uint32_t>(wait.size());
		info.pWaitSemaphores = wait.data();
		info.bufferBindCount = 1;
		info.pBufferBinds = &buffer_memory_bind_info;
		info.imageOpaqueBindCount = 0;
		info.pImageOpaqueBinds = nullptr;
		info.imageBindCount = 0;
		info.pImageBinds = nullptr;
		info.signalSemaphoreCount = static_cast<std::uint32_t>(signal.size());
		info.pSignalSemaphores = signal.data();

		vk_result res = vkQueueBindSparse(queue,
										  1, &info,
										  fence != nullptr ? static_cast<VkFence>(*fence) : vk::vk_null_handle);
		if (!res) {
			throw vk_exception(res);
		}
	}

	VkMemoryRequirements get_memory_requirements() const {
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(Base::device.get(), *this, &req);

		return req;
	}

	std::uint64_t get_elements_count() const override final { return count; }
	std::uint32_t get_element_size_bytes() const override final { return element_size; };
	bool is_sparse() const override final { return true; };
};

}

}
}
