//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_layout.hpp>

namespace StE {
namespace GL {

class device_resource_queue_transferable;

template <int dimensions, class allocation_policy>
class device_image;

class device_image_layout_transformable {
	template <int dimensions, class allocation_policy>
	friend void queue_transfer(device_image<dimensions, allocation_policy> &image,
							   const ste_device_queue::queue_index_t &dst_queue_index,
							   VkPipelineStageFlags src_stage,
							   VkAccessFlags src_access,
							   VkPipelineStageFlags dst_stage,
							   VkAccessFlags dst_access,
							   VkImageLayout dst_layout,
							   bool depth);
	template <int dimensions, class allocation_policy>
	friend void queue_transfer(device_image<dimensions, allocation_policy> &image,
							   const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector,
							   VkPipelineStageFlags src_stage,
							   VkAccessFlags src_access,
							   VkPipelineStageFlags dst_stage,
							   VkAccessFlags dst_access,
							   VkImageLayout dst_layout,
							   bool depth);
	friend class cmd_image_layout_transform;
	friend class cmd_image_layout_transform_discard;

protected:
	mutable device_image_layout image_layout;

protected:
	template <typename... Args>
	device_image_layout_transformable(Args&&... args)
		: image_layout(std::forward<Args>(args)...)
	{}

	device_image_layout_transformable(device_image_layout_transformable&&) = default;
	device_image_layout_transformable &operator=(device_image_layout_transformable&&) = default;

public:
	auto layout() const { return image_layout.layout.load(std::memory_order_acquire); }
};

}
}
