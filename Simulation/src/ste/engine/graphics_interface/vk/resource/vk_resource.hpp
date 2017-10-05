//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_host_allocator.hpp>
#include <vulkan/vulkan.h>
#include <device_memory_heap.hpp>
#include <memory_requirements.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_resource {
public:
	using allocation_t = device_memory_heap::allocation_type;

protected:
	virtual void bind_resource_underlying_memory(const vk_device_memory<host_allocator> &memory, byte_t offset) = 0;

public:
	virtual memory_requirements get_memory_requirements() const = 0;

public:
	vk_resource() = default;
	virtual ~vk_resource() noexcept {}

	vk_resource(vk_resource &&) = default;
	vk_resource& operator=(vk_resource &&) = default;
	vk_resource(const vk_resource &) = delete;
	vk_resource& operator=(const vk_resource &) = delete;

	void bind_memory(const allocation_t &allocation) {
		this->bind_resource_underlying_memory(*allocation.get_memory(),
											  allocation->get_offset());
	}
};

}

}
}
