//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_image_base.hpp>
#include <vk_image_type.hpp>

namespace StE {
namespace GL {

class vk_swapchain_image : public vk_image_base<2> {
	static constexpr int swapchain_image_mips = 1;

public:
	using size_type = vk_image_base<dimensions>::size_type;

public:
	vk_swapchain_image(const vk_logical_device &device,
					   const VkImage &image,
					   const VkFormat &format,
					   const size_type &size,
					   std::uint32_t layers)
		: vk_image_base(device, image, format, size, swapchain_image_mips, layers)
	{}
	~vk_swapchain_image() noexcept {}

	vk_swapchain_image(vk_swapchain_image &&) = default;
	vk_swapchain_image& operator=(vk_swapchain_image &&) = default;
	vk_swapchain_image(const vk_swapchain_image &) = delete;
	vk_swapchain_image& operator=(const vk_swapchain_image &) = delete;
};

}
}
