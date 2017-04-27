//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_image.hpp>

namespace ste {
namespace gl {

class device_image_base
{
protected:
	device_image_base() = default;

public:
	virtual ~device_image_base() noexcept {}

	virtual format get_format() const = 0;
	virtual const vk::vk_image& get_image_handle() const = 0;

	device_image_base(device_image_base&&) = default;
	device_image_base &operator=(device_image_base&&) = default;
};

}
}
