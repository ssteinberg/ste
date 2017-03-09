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

#include <ste_gl_device_memory_allocator.hpp>

namespace StE {
namespace GL {

struct sparse_memory_bind_t {
	const ste_gl_device_memory_allocator::allocation_t *allocation;
	std::uint64_t resource_offset;
};

template <typename T>
class vk_buffer_sparse_impl : public vk_buffer_base {
	using Base = vk_buffer_base;

private:
	std::uint64_t count;

public:
	vk_buffer_sparse_impl(const vk_logical_device &device,
						  std::uint64_t count,
						  const VkBufferUsageFlags &usage)
		: Base(device, count * sizeof(T), usage, true), count(count)
	{}
	~vk_buffer_sparse_impl() noexcept {}

	vk_buffer_sparse_impl(vk_buffer_sparse_impl &&) = default;
	vk_buffer_sparse_impl& operator=(vk_buffer_sparse_impl &&) = default;
	vk_buffer_sparse_impl(const vk_buffer_sparse_impl &) = delete;
	vk_buffer_sparse_impl& operator=(const vk_buffer_sparse_impl &) = delete;

	void bind_memory(const vk_queue &queue,
					 const std::vector<sparse_memory_bind_t> &memory_binds,
					 const std::vector<vk_semaphore*> &wait_semaphores,
					 const std::vector<vk_semaphore*> &signal_semaphores,
					 const vk_fence *fence = nullptr) {
		std::vector<VkSemaphore> wait;
		std::vector<VkSemaphore> signal;
		std::vector<VkSparseMemoryBind> binds;

		if (wait_semaphores.size()) {
			wait.reserve(wait_semaphores.size());
			for (auto &e : wait_semaphores)
				wait.push_back(*e);
		}

		if (signal_semaphores.size()) {
			signal.reserve(signal_semaphores.size());
			for (auto &e : signal_semaphores)
				signal.push_back(*e);
		}

		binds.reserve(memory_binds.size());
		for (auto &e : memory_binds) {
			VkSparseMemoryBind b = {};
			b.resourceOffset = e.resource_offset;
			b.memory = *e.allocation->get_memory();
			b.size = (**e.allocation).get_bytes();
			b.memoryOffset = (**e.allocation).get_offset();
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

	auto& get_elements_count() const { return count; }
};

}
}
