//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_resource_queue_ownership.hpp>

namespace StE {
namespace GL {

class device_resource_queue_transferable;

template <int dimensions, class allocation_policy>
class device_image;

class device_resource_queue_transferable {
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
	friend void queue_transfer_discard(device_resource_queue_transferable &resource,
									   const ste_device_queue::queue_index_t &dst_queue_index);
	friend void queue_transfer_discard(device_resource_queue_transferable &resource,
									   const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector);
	template <int dimensions, class allocation_policy>
	friend void image_layout_transform(device_image<dimensions, allocation_policy> &image,
									   VkPipelineStageFlags src_stage,
									   VkAccessFlags src_access,
									   VkPipelineStageFlags dst_stage,
									   VkAccessFlags dst_access,
									   VkImageLayout dst_layout,
									   bool depth);
	template <int dimensions, class allocation_policy>
	friend void image_layout_transform_discard(device_image<dimensions, allocation_policy> &image,
											   VkPipelineStageFlags src_stage,
											   VkAccessFlags src_access,
											   VkPipelineStageFlags dst_stage,
											   VkAccessFlags dst_access,
											   VkImageLayout dst_layout,
											   bool depth);

protected:
	const ste_context &ctx;

private:
	device_resource_queue_ownership queue_ownership;
	
protected:
	device_resource_queue_transferable(const ste_context &ctx,
									   const device_resource_queue_ownership::resource_queue_selector_t &selector)
		: ctx(ctx),
		queue_ownership(ctx, selector)
	{}
	device_resource_queue_transferable(const ste_context &ctx,
									   const device_resource_queue_ownership::queue_index_t &queue_index)
		: ctx(ctx),
		queue_ownership(queue_index)
	{}

	device_resource_queue_transferable(device_resource_queue_transferable&&) = default;
	device_resource_queue_transferable &operator=(device_resource_queue_transferable&&) = default;

public:
	auto owner_queue_index() const { return queue_ownership.index.load(std::memory_order_acquire); }
};

}
}
