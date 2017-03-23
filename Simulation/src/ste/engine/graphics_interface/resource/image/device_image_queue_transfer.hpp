//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
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
								   std::uint32_t src_family,
								   VkImageLayout src_layout,
								   VkAccessFlags dst_access,
								   std::uint32_t dst_family,
								   VkImageLayout dst_layout,
								   bool depth = false) {
	return vk_image_memory_barrier(image.get(),
								   src_layout,
								   dst_layout,
								   src_access,
								   dst_access,
								   src_family,
								   dst_family,
								   depth);
}

template <int dimensions, class allocation_policy>
void queue_transfer(device_image<dimensions, allocation_policy> &image,
					const ste_device_queue::queue_index_t &dst_queue_index,
					VkPipelineStageFlags src_stage,
					VkAccessFlags src_access,
					VkPipelineStageFlags dst_stage,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout,
					bool depth = false) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	auto& ctx = image.ctx;
	auto src_q_idx = image.owner_queue_index();

	auto& src_q = ctx.device().select_queue(src_q_idx);
	auto& dst_q = ctx.device().select_queue(dst_queue_index);
	
	// Tranfer only if needed
	if (src_q_idx == dst_queue_index) {
		src_q->enqueue([=, &image]() {
			auto src_layout = image.layout();

			auto acquire_batch = ste_device_queue::thread_allocate_batch();
			auto& command_buffer = acquire_batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();

				auto barrier = vk_pipeline_barrier(src_stage,
												   dst_stage,
												   image_layout_transform_barrier(image,
																				 src_access,
																				 src_layout,
																				 dst_access,
																				 dst_layout,
																				 depth));
				recorder << vk_cmd_pipeline_barrier(barrier);
			}
			ste_device_queue::submit_batch(std::move(acquire_batch));

			// Move to new layout on each of the queues
			image.image_layout.layout.store(dst_layout, std::memory_order_relaxed);
		});
		return;
	}

	auto src_family = src_q->queue_descriptor().family;
	auto dst_family = dst_q->queue_descriptor().family;

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

			auto barrier = vk_pipeline_barrier(src_stage,
											   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
											   queue_release_acquire_barrier(image,
																			 src_access,
																			 src_family,
																			 src_layout,
																			 dst_access,
																			 dst_family,
																			 dst_layout,
																			 depth));
			recorder << vk_cmd_pipeline_barrier(barrier);
		}

		ste_device_queue::submit_batch(std::move(release_batch), {}, { *release_batch->user_data() });
		release_acquire_boundary->signal();

		// Move to new layout on each of the queues
		image.image_layout.layout.store(dst_layout, std::memory_order_release);
	});
	dst_q->enqueue([=, &image]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = vk_pipeline_barrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
											   dst_stage,
											   queue_release_acquire_barrier(image,
																			 src_access,
																			 src_family,
																			 dst_layout,
																			 dst_access,
																			 dst_family,
																			 dst_layout,
																			 depth));
			recorder << vk_cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		release_acquire_boundary->get();
		ste_device_queue::submit_batch(std::move(acquire_batch), 
		{ std::make_pair(static_cast<VkSemaphore>(*acquire_batch->user_data()), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) }, {});

		// Move to new layout on each of the queues
		image.image_layout.layout.store(dst_layout, std::memory_order_relaxed);
	});

	image.queue_ownership.index.store(dst_queue_index, std::memory_order_release);
}

template <int dimensions, class allocation_policy>
void queue_transfer(device_image<dimensions, allocation_policy> &image,
					const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector,
					VkPipelineStageFlags src_stage,
					VkAccessFlags src_access,
					VkPipelineStageFlags dst_stage,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout,
					bool depth = false) {
	auto& dst_q = image.ctx.device().select_queue(queue_selector);
	auto dst_q_idx = dst_q->index();

	queue_transfer(image,
				   dst_q_idx,
				   src_stage,
				   src_access,
				   dst_stage,
				   dst_access,
				   dst_layout,
				   depth);
}

void inline queue_transfer_discard(device_resource_queue_transferable &resource,
								   const ste_device_queue::queue_index_t &dst_queue_index) {
	resource.queue_ownership.index.store(dst_queue_index, std::memory_order_release);
}

void inline queue_transfer_discard(device_resource_queue_transferable &resource,
								   const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector) {
	auto& dst_q = resource.ctx.device().select_queue(queue_selector);
	auto dst_q_idx = dst_q->index();

	queue_transfer_discard(resource, dst_q_idx);
}

}
}
