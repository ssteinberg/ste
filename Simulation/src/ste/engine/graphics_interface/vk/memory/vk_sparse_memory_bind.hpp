//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_memory_allocator.hpp>

namespace ste {
namespace gl {

namespace vk {

struct vk_sparse_memory_bind {
	ste_gl_device_memory_allocator::allocation_t *allocation;
	std::uint64_t resource_offset_bytes;
	std::uint64_t size_bytes;
};

}

}
}
