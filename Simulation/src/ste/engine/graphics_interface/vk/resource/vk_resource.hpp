//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <device_memory_heap.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_resource {
public:
	using allocation_t = device_memory_heap::allocation_type;

protected:
	virtual void bind_resource_underlying_memory(const vk_device_memory &memory, std::uint64_t offset) = 0;

public:
	virtual VkMemoryRequirements get_memory_requirements() const = 0;

public:
	vk_resource() = default;
	virtual ~vk_resource() noexcept {}

	vk_resource(vk_resource &&) = default;
	vk_resource& operator=(vk_resource &&) = default;
	vk_resource(const vk_resource &) = delete;
	vk_resource& operator=(const vk_resource &) = delete;

	void bind_memory(const allocation_t &allocation) {
		this->bind_resource_underlying_memory(*allocation.get_memory(),
											  allocation.get()->get_offset());
	}
};

}

}
}
