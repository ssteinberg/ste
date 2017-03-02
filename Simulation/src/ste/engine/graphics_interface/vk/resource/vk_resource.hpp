//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_device_memory.hpp>

namespace StE {
namespace GL {

class vk_resource {
private:
	const vk_device_memory *memory{ nullptr };
	bool private_underlying_memory{ false };

protected:
	const vk_logical_device &device;

protected:
	virtual void bind_resource_underlying_memory(const vk_device_memory &memory, std::uint64_t offset) = 0;

public:
	virtual VkMemoryRequirements get_memory_requirements() const = 0;

public:
	vk_resource(const vk_logical_device &device) : device(device) {}
	virtual ~vk_resource() noexcept {}

	vk_resource(vk_resource &&) = default;
	vk_resource& operator=(vk_resource &&) = default;
	vk_resource(const vk_resource &) = delete;
	vk_resource& operator=(const vk_resource &) = delete;

	void bind_memory(const vk_device_memory *memory, bool private_memory, std::uint64_t offset) {
		this->private_underlying_memory = private_memory;
		this->memory = memory;
		this->bind_resource_underlying_memory(*memory, offset);
	}

	auto& get_creating_device() const { return device; }
	auto& get_underlying_memory() const { return memory; }
	auto has_bounded_underlying_memory() const { return !!memory; }
	auto has_private_underlying_memory() const { return private_underlying_memory; }
};

}
}
