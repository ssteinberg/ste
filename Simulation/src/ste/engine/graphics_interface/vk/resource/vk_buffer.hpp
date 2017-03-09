//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_resource.hpp>
#include <vk_buffer_base.hpp>
#include <vk_buffer_sparse_impl.hpp>

#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_device_memory.hpp>

#include <type_traits>

namespace StE {
namespace GL {

template <typename T>
class vk_buffer_impl : public vk_buffer_base, public vk_resource {
	using Base = vk_buffer_base;

private:
	std::uint64_t count;

protected:
	void bind_resource_underlying_memory(const vk_device_memory &memory, std::uint64_t offset) override {
		vk_result res = vkBindBufferMemory(Base::device, *this, memory, offset);
		if (!res) {
			throw vk_exception(res);
		}
	}

public:
	vk_buffer_impl(const vk_logical_device &device,
				   std::uint64_t count,
				   const VkBufferUsageFlags &usage)
		: Base(device, count * sizeof(T), usage, false), count(count)
	{}
	~vk_buffer_impl() noexcept {}

	vk_buffer_impl(vk_buffer_impl &&) = default;
	vk_buffer_impl& operator=(vk_buffer_impl &&) = default;
	vk_buffer_impl(const vk_buffer_impl &) = delete;
	vk_buffer_impl& operator=(const vk_buffer_impl &) = delete;

	VkMemoryRequirements get_memory_requirements() const override {
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(Base::device, *this, &req);

		return req;
	}

	auto& get_elements_count() const { return count; }
};

template <typename T, bool Sparse = false>
class vk_buffer : public std::conditional_t<Sparse, vk_buffer_sparse_impl<T>, vk_buffer_impl<T>> {
	using Base = std::conditional_t<Sparse, vk_buffer_sparse_impl<T>, vk_buffer_impl<T>>;

public:	
	using Base::Base;
};

}
}
