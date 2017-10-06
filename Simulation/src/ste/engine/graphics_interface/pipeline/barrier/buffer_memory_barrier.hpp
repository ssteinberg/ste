//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <device_buffer_base.hpp>

#include <ste_queue_family.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

class buffer_memory_barrier {
	friend class cmd_pipeline_barrier;

private:
	access_flags src;
	access_flags dst;
	ste_queue_family src_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	ste_queue_family dst_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	std::reference_wrapper<const device_buffer_base> buffer;
	byte_t offset{ 0_B };
	byte_t size{ ~0_B };

public:
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const access_flags &src_access,
						  const access_flags &dst_access)
		: src(src_access), dst(dst_access), buffer(buffer)
	{}
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const access_flags &src_access,
						  const access_flags &dst_access,
						  std::uint64_t size,
						  std::uint64_t offset = 0)
		: src(src_access), dst(dst_access),
		buffer(buffer),
		offset(offset * buffer.get_element_size_bytes()), size(size * buffer.get_element_size_bytes())
	{}
	buffer_memory_barrier(const device_buffer_base &buffer,
						  const access_flags &src_access,
						  const access_flags &dst_access,
						  const ste_queue_family &src_queue_family,
						  const ste_queue_family &dst_queue_family)
		: src(src_access), dst(dst_access),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		buffer(buffer)
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
		barrier.srcAccessMask = static_cast<VkAccessFlags>(src);
		barrier.dstAccessMask = static_cast<VkAccessFlags>(dst);
		barrier.srcQueueFamilyIndex = static_cast<std::uint32_t>(src_queue_family);
		barrier.dstQueueFamilyIndex = static_cast<std::uint32_t>(dst_queue_family);
		barrier.buffer = buffer.get().get_buffer_handle();
		barrier.offset = static_cast<std::size_t>(offset);
		barrier.size = static_cast<std::size_t>(size);

		return barrier;
	}
};

}
}
