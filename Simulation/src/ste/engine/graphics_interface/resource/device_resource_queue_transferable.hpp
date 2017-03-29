//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <ste_context_predefine.hpp>
#include <device_resource_queue_ownership.hpp>

namespace StE {

namespace GL {

class device_image_base;
class device_buffer_base;

class device_resource_queue_transferable {
	friend void queue_transfer(const ste_context &ctx,
							   device_image_base &image,
							   const ste_queue_family &dst_family,
							   VkPipelineStageFlags src_stage,
							   VkAccessFlags src_access,
							   VkPipelineStageFlags dst_stage,
							   VkAccessFlags dst_access,
							   VkImageLayout dst_layout,
							   bool depth);
	friend void queue_transfer(const ste_context &ctx,
							   device_buffer_base &buffer,
							   const ste_queue_family &dst_family,
							   VkPipelineStageFlags src_stage,
							   VkAccessFlags src_access,
							   VkPipelineStageFlags dst_stage,
							   VkAccessFlags dst_access);
	friend void queue_transfer_discard(device_resource_queue_transferable &resource,
									   const ste_queue_family &dst_family);
	friend class cmd_pipeline_barrier;

private:
	mutable device_resource_queue_ownership queue_ownership;
	
protected:
	device_resource_queue_transferable(const device_resource_queue_ownership::family_t &family)
		: queue_ownership(family)
	{}

	device_resource_queue_transferable(device_resource_queue_transferable&&) = default;
	device_resource_queue_transferable &operator=(device_resource_queue_transferable&&) = default;

public:
	auto owner_queue_family() const { return queue_ownership.family.load(std::memory_order_acquire); }
};

}
}
