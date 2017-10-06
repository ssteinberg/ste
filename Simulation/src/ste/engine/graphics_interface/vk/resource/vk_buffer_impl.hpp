//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_resource.hpp>
#include <vk_buffer.hpp>

#include <vk_host_allocator.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_device_memory.hpp>

namespace ste {
namespace gl {

namespace vk {

namespace _detail {

template <bool sparse, typename host_allocator = vk_host_allocator<>>
class vk_buffer_impl : public vk_buffer<host_allocator>, public vk_resource<host_allocator> {
	using Base = vk_buffer<host_allocator>;

private:
	byte_t element_size;
	std::uint64_t count;

protected:
	void bind_resource_underlying_memory(const vk_device_memory<host_allocator> &memory, byte_t offset) override {
		const vk_result res = vkBindBufferMemory(Base::device.get(), 
												 *this, 
												 memory, 
												 static_cast<std::size_t>(offset));
		if (!res) {
			throw vk_exception(res);
		}
	}

public:
	vk_buffer_impl(const vk_logical_device<host_allocator> &device,
				   byte_t element_size,
				   std::uint64_t count,
				   const VkBufferUsageFlags &usage,
				   const char *name)
		: Base(device,
			   count * element_size,
			   usage,
			   sparse,
			   name),
		  vk_resource(),
		  element_size(element_size),
		  count(count) {}

	~vk_buffer_impl() noexcept {}

	vk_buffer_impl(vk_buffer_impl &&) = default;
	vk_buffer_impl &operator=(vk_buffer_impl &&) = default;
	vk_buffer_impl(const vk_buffer_impl &) = delete;
	vk_buffer_impl &operator=(const vk_buffer_impl &) = delete;

	memory_requirements get_memory_requirements() const override {
		VkBufferMemoryRequirementsInfo2KHR info = {};
		VkMemoryDedicatedRequirementsKHR dedicated_info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR;
		info.buffer = *this;
		info.pNext = &dedicated_info;
		dedicated_info.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
		dedicated_info.pNext = nullptr;

		VkMemoryRequirements2KHR req;
		req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
		req.pNext = nullptr;
		this->device.get().get_extensions_func_pointers().get_memory_requirements2().vkGetBufferMemoryRequirements2KHR(this->device.get(),
																													   &info,
																													   &req);

		return memory_requirements(req, 
								   dedicated_info);
	}

	std::uint64_t get_elements_count() const override final { return count; }
	byte_t get_element_size_bytes() const override final { return element_size; };
	bool is_sparse() const override final { return sparse; };
};

}

}

}
}
