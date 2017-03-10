//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_memory_allocator.hpp>

namespace StE {
namespace GL {

struct vk_sparse_memory_bind {
	ste_gl_device_memory_allocator::allocation_t allocation;
	std::uint64_t resource_offset{ 0 };

	vk_sparse_memory_bind() = default;
	vk_sparse_memory_bind(ste_gl_device_memory_allocator::allocation_t &&allocation,
						  std::uint64_t resource_offset) : allocation(std::move(allocation)), resource_offset(resource_offset) {}

	vk_sparse_memory_bind(vk_sparse_memory_bind &&) = default;
	vk_sparse_memory_bind &operator=(vk_sparse_memory_bind &&) = default;
};

}
}
