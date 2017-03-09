//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class vk_buffer_memory_barrier {
private:
	VkAccessFlags src;
	VkAccessFlags dst;
	std::uint32_t src_queue_index{ VK_QUEUE_FAMILY_IGNORED };
	std::uint32_t dst_queue_index{ VK_QUEUE_FAMILY_IGNORED };
	VkBuffer buffer;
	std::uint64_t offset{ 0 };
	std::uint64_t size{ VK_WHOLE_SIZE };

public:
	vk_buffer_memory_barrier(const vk_buffer_base &buffer,
							 const VkAccessFlags &src_access,
							 const VkAccessFlags &dst_access)
		: src(src_access), dst(dst_access), buffer(buffer)
	{}
	vk_buffer_memory_barrier(const vk_buffer_base &buffer,
							 const VkAccessFlags &src_access,
							 const VkAccessFlags &dst_access,
							 std::uint64_t size,
							 std::uint64_t offset = 0)
		: src(src_access), dst(dst_access), 
		buffer(buffer), offset(offset), size(size)
	{}
	vk_buffer_memory_barrier(const vk_buffer_base &buffer,
							 const VkAccessFlags &src_access,
							 const VkAccessFlags &dst_access,
							 std::uint32_t src_queue_index,
							 std::uint32_t dst_queue_index,
							 std::uint64_t size,
							 std::uint64_t offset = 0)
		: src(src_access), dst(dst_access),
		src_queue_index(src_queue_index), dst_queue_index(dst_queue_index), 
		buffer(buffer), offset(offset), size(size)
	{}
	~vk_buffer_memory_barrier() noexcept {}

	vk_buffer_memory_barrier(vk_buffer_memory_barrier &&) = default;
	vk_buffer_memory_barrier &operator=(vk_buffer_memory_barrier &&) = default;
	vk_buffer_memory_barrier(const vk_buffer_memory_barrier &) = default;
	vk_buffer_memory_barrier &operator=(const vk_buffer_memory_barrier &) = default;

	auto &get_src() const { return src; };
	auto &get_dst() const { return dst; };

	operator VkBufferMemoryBarrier() const {
		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = src;
		barrier.dstAccessMask = dst;
		barrier.srcQueueFamilyIndex = src_queue_index;
		barrier.dstQueueFamilyIndex = dst_queue_index;
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;

		return barrier;
	}
};

}
}
