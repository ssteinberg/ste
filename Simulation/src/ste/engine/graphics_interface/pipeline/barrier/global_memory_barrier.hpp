//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class global_memory_barrier {
private:
	VkAccessFlags src;
	VkAccessFlags dst;

public:
	global_memory_barrier(const VkAccessFlags &src_access,
							 const VkAccessFlags &dst_access)
		: src(src_access), dst(dst_access) {}
	~global_memory_barrier() noexcept {}

	global_memory_barrier(global_memory_barrier &&) = default;
	global_memory_barrier &operator=(global_memory_barrier &&) = default;
	global_memory_barrier(const global_memory_barrier &) = default;
	global_memory_barrier &operator=(const global_memory_barrier &) = default;

	auto &get_src() const { return src; };
	auto &get_dst() const { return dst; };

	operator VkMemoryBarrier() const {
		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = src;
		barrier.dstAccessMask = dst;

		return barrier;
	}
};

}
}
