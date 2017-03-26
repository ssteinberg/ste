//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_queue_transferable.hpp>

namespace StE {
namespace GL {

template <typename T>
class device_buffer_base 
	: public device_resource_queue_transferable {
protected:
	template <typename selector_policy>
	device_buffer_base(const ste_context &ctx,
					   const ste_queue_selector<selector_policy> &selector)
		: device_resource_queue_transferable(ctx, selector)
	{}
	device_buffer_base(const ste_context &ctx,
					   const device_resource_queue_ownership::family_t &family)
		: device_resource_queue_transferable(ctx, family)
	{}

public:
	virtual ~device_buffer_base() noexcept {}
	virtual VkBuffer get_buffer_handle() const = 0;

	device_buffer_base(device_buffer_base&&) = default;
	device_buffer_base &operator=(device_buffer_base&&) = default;
};

}
}
