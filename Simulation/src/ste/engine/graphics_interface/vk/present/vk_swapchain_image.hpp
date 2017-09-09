//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_image.hpp>
#include <image_type.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_swapchain_image : public vk_image<host_allocator> {
	using Base = vk_image<host_allocator>;
	static constexpr int swapchain_image_mips = 1;

public:
	using extent_type = glm::uvec2;

private:
	using Base::Base;
	using Base::get_memory_requirements;
	using Base::bind_resource_underlying_memory;

public:
	vk_swapchain_image(const vk_logical_device<host_allocator> &device,
					   const VkImage &image,
					   const VkFormat &image_format,
					   const extent_type &size,
					   std::uint32_t layers,
					   const char *name)
		: Base(device,
			   name,
			   image,
			   image_format,
			   Base::extent_type{ size.x, size.y, 1 },
			   0,
			   swapchain_image_mips, 
			   layers)
	{}
	~vk_swapchain_image() noexcept {
		this->image = none;
	}

	vk_swapchain_image(vk_swapchain_image &&) = default;
	vk_swapchain_image& operator=(vk_swapchain_image &&) = default;
	vk_swapchain_image(const vk_swapchain_image &) = delete;
	vk_swapchain_image& operator=(const vk_swapchain_image &) = delete;
};

}

}
}
