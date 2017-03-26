//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_queue_transferable.hpp>
#include <device_image_layout_transformable.hpp>

namespace StE {
namespace GL {

template <int dimensions>
class device_image_base
	: public device_resource_queue_transferable,
	public device_image_layout_transformable 
{
protected:
	template <typename selector_policy>
	device_image_base(const ste_context &ctx,
					  const ste_queue_selector<selector_policy> &selector,
					  const vk_image_initial_layout &layout)
		: device_resource_queue_transferable(ctx, selector),
		device_image_layout_transformable(layout)
	{}
	device_image_base(const ste_context &ctx,
					  const device_resource_queue_ownership::family_t &family,
					  const vk_image_initial_layout &layout)
		: device_resource_queue_transferable(ctx, family),
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
