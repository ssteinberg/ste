//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <device_memory_heap.hpp>

namespace StE {
namespace GL {

class vk_resource {
public:
	using allocation_t = device_memory_heap::allocation_type;

private:
	allocation_t allocation;
	bool private_underlying_memory{ false };

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

	auto& get_underlying_memory() const { return *this->allocation.get_memory(); }
	bool has_bounded_underlying_memory() const { return allocation; }
	bool has_private_underlying_memory() const { return private_underlying_memory; }

	void bind_memory(allocation_t &&allocation,
					 bool private_memory) {
		this->private_underlying_memory = private_memory;
		this->allocation = std::move(allocation);
		this->bind_resource_underlying_memory(get_underlying_memory(),
											  this->allocation.get()->get_offset());
	}
};

}
}
