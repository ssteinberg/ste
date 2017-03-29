//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_queue_transferable.hpp>
#include <device_image_layout_transformable.hpp>

namespace StE {
namespace GL {

class device_image_base
	: public device_resource_queue_transferable,
	public device_image_layout_transformable 
{
protected:
	device_image_base(const device_resource_queue_ownership::family_t &family,
					  const vk_image_initial_layout &layout)
		: device_resource_queue_transferable(family),
		device_image_layout_transformable(layout)
	{}

public:
	virtual ~device_image_base() noexcept {}
	virtual VkImage get_image_handle() const = 0;

	device_image_base(device_image_base&&) = default;
	device_image_base &operator=(device_image_base&&) = default;
};

}
}
