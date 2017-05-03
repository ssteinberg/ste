//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_swapchain_image.hpp>
#include <device_image_base.hpp>

namespace ste {
namespace gl {

class device_swapchain_image : public device_image_base {
private:
	vk::vk_swapchain_image image;

public:
	device_swapchain_image(vk::vk_swapchain_image &&image)
		: image(std::move(image))
	{}

	device_swapchain_image(device_swapchain_image&&) = default;
	device_swapchain_image &operator=(device_swapchain_image&&) = default;

	const glm::u32vec3& get_extent() const override final { return get_image_handle().get_extent(); }
	format get_format() const override final { return static_cast<format>(image.get_format()); }
	const vk::vk_image& get_image_handle() const override final { return image; };
};

}
}
