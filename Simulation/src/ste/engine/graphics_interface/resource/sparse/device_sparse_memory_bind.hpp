//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_memory_allocator.hpp>

namespace ste {
namespace gl {

struct device_sparse_memory_bind {
	ste_gl_device_memory_allocator::allocation_t *allocation{ nullptr };
	std::uint64_t resource_offset_bytes;
	std::uint64_t size_bytes;
};

}
}
