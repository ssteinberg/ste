//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <ste_queue_family.hpp>
#include <device_buffer_base.hpp>
#include <device_resource_queue_transferable.hpp>

namespace StE {
namespace GL {

class buffer_memory_barrier {
	friend class cmd_pipeline_barrier;

private:
	VkAccessFlags src;
	VkAccessFlags dst;
	ste_queue_family src_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	ste_queue_family dst_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	VkBuffer buffer;
	std::uint64_t offset{ 0 };
	std::uint64_t size{ VK_WHOLE_SIZE };

	const device_resource_queue_transferable *queue_ownership;

public:
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const VkAccessFlags &src_access,
						  const VkAccessFlags &dst_access)
		: src(src_access), dst(dst_access), buffer(buffer.get_buffer_handle()),
		queue_ownership(&buffer)
	{}
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const VkAccessFlags &src_access,
						  const VkAccessFlags &dst_access,
						  std::uint64_t size,
						  std::uint64_t offset = 0)
		: src(src_access), dst(dst_access),
		buffer(buffer.get_buffer_handle()), 
		offset(offset * buffer.get_element_size_bytes()), size(size * buffer.get_element_size_bytes()),
		queue_ownership(&buffer)
	{}
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const VkAccessFlags &src_access,
						  const VkAccessFlags &dst_access,
						  const ste_queue_family &src_queue_family,
						  const ste_queue_family &dst_queue_family)
		: src(src_access), dst(dst_access),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		buffer(buffer.get_buffer_handle()), queue_ownership(&buffer)
	{}
	~buffer_memory_barrier() noexcept {}

	buffer_memory_barrier(buffer_memory_barrier &&) = default;
	buffer_memory_barrier &operator=(buffer_memory_barrier &&) = default;
	buffer_memory_barrier(const buffer_memory_barrier &) = default;
	buffer_memory_barrier &operator=(const buffer_memory_barrier &) = default;

	operator VkBufferMemoryBarrier() const {
		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = src;
		barrier.dstAccessMask = dst;
		barrier.srcQueueFamilyIndex = static_cast<std::uint32_t>(src_queue_family);
		barrier.dstQueueFamilyIndex = static_cast<std::uint32_t>(dst_queue_family);
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;

		return barrier;
	}
};

}
}
