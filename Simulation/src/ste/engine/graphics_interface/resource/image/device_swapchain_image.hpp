//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_swapchain_image.hpp>
#include <device_image_base.hpp>

namespace StE {
namespace GL {

class device_swapchain_image : public device_image_base {
private:
	vk_swapchain_image image;

public:
	device_swapchain_image(vk_swapchain_image &&image)
		: device_image_base(0,
							vk_image_initial_layout::unused),
		image(std::move(image))
	{}

	device_swapchain_image(device_swapchain_image&&) = default;
	device_swapchain_image &operator=(device_swapchain_image&&) = default;

	VkFormat get_format() const override final { return image.get_format(); }
	VkImage get_image_handle() const override final { return image; };
};

}
}
