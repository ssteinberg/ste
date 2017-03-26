//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_buffer.hpp>
#include <device_buffer_base.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

namespace StE {
namespace GL {

template <typename T, class allocation_policy = device_resource_allocation_policy_device>
class device_buffer
	: public device_buffer_base<T>,
	public device_resource<vk_buffer<T>, allocation_policy>
{
public:
	template <typename selector_policy, typename ... Args>
	device_buffer(const ste_context &ctx,
				  const ste_queue_selector<selector_policy> &initial_queue_selector,
				  Args&&... args)
		: device_buffer_base(ctx, initial_queue_selector),
		device_resource(ctx,
						std::forward<Args>(args)...)
	{}
	template <typename ... Args>
	device_buffer(const ste_context &ctx,
				  const device_resource_queue_ownership::family_t &family,
				  Args&&... args)
		: device_buffer_base(ctx, family),
		device_resource(ctx,
						std::forward<Args>(args)...)
	{}
	~device_buffer() noexcept {}
	
	VkBuffer get_buffer_handle() const override final { return *this; }

	device_buffer(device_buffer&&) = default;
	device_buffer &operator=(device_buffer&&) = default;
};

}
}
