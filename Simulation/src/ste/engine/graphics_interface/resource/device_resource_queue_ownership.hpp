//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_device_queue.hpp>

#include <atomic>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy>
class device_image;

class device_resource_queue_ownership {
	template <int dimensions, class allocation_policy>
	friend void queue_transfer(device_image<dimensions, allocation_policy> &,
						const ste_device_queue::queue_index_t &,
						VkAccessFlags,
						VkAccessFlags,
						VkImageLayout);
	template <int dimensions, class allocation_policy>
	friend void queue_transfer(device_image<dimensions, allocation_policy> &,
							   const ste_queue_selector<ste_queue_selector_default_policy> &,
							   VkAccessFlags,
							   VkAccessFlags,
							   VkImageLayout);
	template <int dimensions, class allocation_policy>
	friend void queue_transfer_discard(device_image<dimensions, allocation_policy> &,
									   const ste_queue_selector<ste_queue_selector_default_policy> &);
	template <int dimensions, class allocation_policy>
	friend void queue_transfer_discard(device_image<dimensions, allocation_policy> &,
									   const ste_device_queue::queue_index_t &);

public:
	using queue_index_t = ste_device_queue::queue_index_t;
	using resource_queue_selector_t = const ste_queue_selector<ste_queue_selector_default_policy> &;

private:
	queue_index_t queue_index;

public:
	device_resource_queue_ownership() = delete;
	device_resource_queue_ownership(const ste_context &ctx,
									const resource_queue_selector_t &selector)
		: queue_index(ctx.device().select_queue(selector)->index())
	{}
	device_resource_queue_ownership(const queue_index_t &index)
		: queue_index(index)
	{}

	device_resource_queue_ownership(device_resource_queue_ownership &&o) = default;
	device_resource_queue_ownership &operator=(device_resource_queue_ownership &&o) = default;

	auto index() const { return queue_index; }
};

}
}
