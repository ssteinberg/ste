//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_queue_family.hpp>
#include <device_image_base.hpp>
#include <device_image_layout_transform.hpp>

#include <pipeline_stage.hpp>

#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <image_memory_barrier.hpp>

#include <boundary.hpp>
#include <memory>

namespace ste {
namespace gl {

auto inline queue_release_acquire_barrier(const device_image_base &image,
										  VkAccessFlags src_access,
										  const ste_queue_family &src_family,
										  VkImageLayout src_layout,
										  VkAccessFlags dst_access,
										  const ste_queue_family &dst_family,
										  VkImageLayout dst_layout) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access,
								src_family,
								dst_family);
}

void inline queue_transfer(const ste_context &ctx,
						   device_image_base &image,
						   ste_device_queue &src_queue,
						   ste_device_queue &dst_queue,
						   VkImageLayout src_layout,
						   pipeline_stage src_stage,
						   VkAccessFlags src_access,
						   pipeline_stage dst_stage,
						   VkAccessFlags dst_access,
						   VkImageLayout dst_layout) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	auto src_family = src_queue.queue_descriptor().family;
	auto dst_family = dst_queue.queue_descriptor().family;

	// Tranfer only if needed
	if (src_family == dst_family) {
		src_queue.enqueue([=, &image]() {
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
																			   dst_layout));
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

	src_queue.enqueue([=, &image]() {
		auto release_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(src_stage,
											pipeline_stage::top_of_pipe,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  src_family,
																		  src_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		ste_device_queue::submit_batch(std::move(release_batch), {}, { *release_batch->user_data() });
		release_acquire_boundary->signal();
	});
	dst_queue.enqueue([=, &image]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(pipeline_stage::top_of_pipe,
											dst_stage,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  src_family,
																		  dst_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		release_acquire_boundary->get();
		ste_device_queue::submit_batch(std::move(acquire_batch),
					{ std::make_pair(static_cast<VkSemaphore>(*acquire_batch->user_data()), pipeline_stage::bottom_of_pipe) }, {});
	});
}

void inline queue_transfer_discard(const ste_context &ctx,
								   device_image_base &image,
								   const ste_queue_selector<ste_queue_selector_policy_strict> &dst_queue_selector,
								   pipeline_stage stage,
								   VkImageLayout src_layout,
								   VkAccessFlags src_access,
								   VkImageLayout dst_layout,
								   VkAccessFlags dst_access) {
	auto &dst_queue = *ctx.device().select_queue(dst_queue_selector);
	ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	dst_queue.enqueue([=, &image]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											stage,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  0,
																		  src_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		ste_device_queue::submit_batch(std::move(acquire_batch));
	});
}

}
}
