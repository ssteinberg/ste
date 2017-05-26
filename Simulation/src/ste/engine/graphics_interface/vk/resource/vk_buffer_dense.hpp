//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_resource.hpp>
#include <vk_buffer.hpp>

#include <vk_host_allocator.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_device_memory.hpp>

#include <type_traits>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_buffer_dense : public vk_buffer<host_allocator>, public vk_resource<host_allocator> {
	using Base = vk_buffer<host_allocator>;

private:
	std::uint32_t element_size;
	std::uint64_t count;

protected:
	void bind_resource_underlying_memory(const vk_device_memory<host_allocator> &memory, std::uint64_t offset) override {
		vk_result res = vkBindBufferMemory(Base::device.get(), *this, memory, offset);
		if (!res) {
			throw vk_exception(res);
		}
	}

public:
	vk_buffer_dense(const vk_logical_device<host_allocator> &device,
					std::uint32_t element_size,
					std::uint64_t count,
					const VkBufferUsageFlags &usage)
		: Base(device, count * element_size, usage, false),
		vk_resource(),
		element_size(element_size),
		count(count)
	{}
	~vk_buffer_dense() noexcept {}

	vk_buffer_dense(vk_buffer_dense &&) = default;
	vk_buffer_dense& operator=(vk_buffer_dense &&) = default;
	vk_buffer_dense(const vk_buffer_dense &) = delete;
	vk_buffer_dense& operator=(const vk_buffer_dense &) = delete;

	VkMemoryRequirements get_memory_requirements() const override {
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(Base::device.get(), *this, &req);

		return req;
	}

	std::uint64_t get_elements_count() const override final { return count; }
	std::uint32_t get_element_size_bytes() const override final { return element_size; };
	bool is_sparse() const override final { return false; };
};

}

}
}
