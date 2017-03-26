//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_family.hpp>
#include <device_image.hpp>
#include <device_image_layout_transform.hpp>
#include <device_resource_queue_ownership.hpp>

#include <boundary.hpp>
#include <memory>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy>
auto queue_release_acquire_barrier(const device_image<dimensions, allocation_policy> &image,
								   VkAccessFlags src_access,
								   const ste_queue_family &src_family,
								   VkImageLayout src_layout,
								   VkAccessFlags dst_access,
								   const ste_queue_family &dst_family,
								   VkImageLayout dst_layout,
								   bool depth = false) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access,
								src_family,
								dst_family,
								depth);
}

template <int dimensions, class allocation_policy>
void queue_transfer(const ste_context &ctx,
					device_image<dimensions, allocation_policy> &image,
					const ste_queue_family &dst_family,
					VkPipelineStageFlags src_stage,
					VkAccessFlags src_access,
					VkPipelineStageFlags dst_stage,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout,
					bool depth = false) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	auto src_family = image.owner_queue_family();

	auto& src_q = ctx.device().select_queue(make_family_queue_selector(src_family));
	auto& dst_q = ctx.device().select_queue(make_family_queue_selector(dst_family));
	
	// Tranfer only if needed
	if (src_family == dst_family) {
		src_q->enqueue([=, &image]() {
			auto src_layout = image.layout();

			auto acquire_batch = ste_device_queue::thread_allocate_batch();
			auto& command_buffer = acquire_batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();

				auto barrier = pipeline_barrier(src_stage,
												   dst_stage,
												   image_layout_transform_barrier(image,
																				 src_access,
																				 src_layout,
																				 dst_access,
																				 dst_layout,
																				 depth));
				recorder << cmd_pipeline_barrier(barrier);
			}
			ste_device_queue::submit_batch(std::move(acquire_batch));
		});
		return;
	}

	// Get a semaphore and boundary
	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;
	using user_data_t = std::shared_ptr<semaphore_t>;
	auto user_data = std::make_shared<semaphore_t>(ctx.device().get_sync_primitives_pools().semaphores().claim());
	auto release_acquire_boundary = std::make_shared<boundary<void>>();

	src_q->enqueue([=, &image]() {
		auto src_layout = image.layout();

		auto release_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(src_stage,
											VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  src_family,
																		  src_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout,
																		  depth));
			recorder << cmd_pipeline_barrier(barrier);
		}

		ste_device_queue::submit_batch(std::move(release_batch), {}, { *release_batch->user_data() });
		release_acquire_boundary->signal();
	});
	dst_q->enqueue([=, &image]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
											dst_stage,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  src_family,
																		  dst_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout,
																		  depth));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		release_acquire_boundary->get();
		ste_device_queue::submit_batch(std::move(acquire_batch), 
		{ std::make_pair(static_cast<VkSemaphore>(*acquire_batch->user_data()), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) }, {});
	});

	image.queue_ownership.family.store(dst_family, std::memory_order_release);
}

template <int dimensions, class allocation_policy, typename selector_policy>
void queue_transfer(const ste_context &ctx,
					device_image<dimensions, allocation_policy> &image,
					const ste_queue_selector<selector_policy> &queue_selector,
					VkPipelineStageFlags src_stage,
					VkAccessFlags src_access,
					VkPipelineStageFlags dst_stage,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout,
					bool depth = false) {
	auto& dst_q = ctx.device().select_queue(queue_selector);
	auto dst_family = dst_q->queue_descriptor().family;

	queue_transfer(ctx,
				   image,
				   dst_family,
				   src_stage,
				   src_access,
				   dst_stage,
				   dst_access,
				   dst_layout,
				   depth);
}

void inline queue_transfer_discard(device_resource_queue_transferable &resource,
								   const ste_queue_family &dst_family) {
	resource.queue_ownership.family.store(dst_family, std::memory_order_release);
}

template <typename selector_policy>
void queue_transfer_discard(const ste_context &ctx,
							device_resource_queue_transferable &resource,
							const ste_queue_selector<selector_policy> &queue_selector) {
	auto& dst_q = ctx.device().select_queue(queue_selector);
	auto dst_family = dst_q->queue_descriptor().family;

	queue_transfer_discard(resource, dst_family);
}

}
}
