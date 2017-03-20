//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_image.hpp>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy>
class device_image;

class device_image_layout {
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

private:
	VkImageLayout image_layout;

public:
	device_image_layout() = delete;
	device_image_layout(const vk_image_initial_layout &layout)
		: image_layout(layout == vk_image_initial_layout::preinitialized ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED)
	{}
	device_image_layout(const VkImageLayout &layout)
	: image_layout(layout)
	{}

	device_image_layout(device_image_layout &&o) = default;
	device_image_layout &operator=(device_image_layout &&o) = default;

	auto layout() const { return image_layout; }
};

}
}
