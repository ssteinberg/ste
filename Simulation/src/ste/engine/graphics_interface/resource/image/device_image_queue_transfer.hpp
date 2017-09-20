//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_queue_family.hpp>
#include <device_image_base.hpp>
#include <device_image_layout_transform.hpp>

#include <pipeline_stage.hpp>
#include <image_layout.hpp>
#include <access_flags.hpp>

#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <image_memory_barrier.hpp>

#include <boundary.hpp>
#include <lib/unique_ptr.hpp>
#include <future>

namespace ste {
namespace gl {

auto inline queue_release_acquire_barrier(const device_image_base &image,
										  access_flags src_access,
										  const ste_queue_family &src_family,
										  image_layout src_layout,
										  access_flags dst_access,
										  const ste_queue_family &dst_family,
										  image_layout dst_layout) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access,
								src_family,
								dst_family);
}

/**
*	@brief	Enqueues image queue ownership transfer and image layout transform, perserving old content.
*	
 *	@return	Enqueue future.
*/
auto inline queue_transfer(const ste_context &ctx,
						   const device_image_base &image,
						   ste_device_queue &src_queue,
						   ste_device_queue &dst_queue,
						   image_layout src_layout,
						   pipeline_stage src_stage,
						   access_flags src_access,
						   image_layout dst_layout,
						   pipeline_stage dst_stage,
						   access_flags dst_access,
						   lib::vector<wait_semaphore> &&wait_semaphores = {},
						   lib::vector<const semaphore*> &&signal_semaphores = {}) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	auto src_family = src_queue.queue_descriptor().family;
	auto dst_family = dst_queue.queue_descriptor().family;

	// Tranfer only if needed
	if (src_family == dst_family) {
		if (src_layout == dst_layout) {
			// Nothing to do
			return std::async(std::launch::deferred, []() {});
		}

		// Only transform layout
		auto future = src_queue.enqueue([=, &image, signal_semaphores = std::move(signal_semaphores), wait_semaphores = std::move(wait_semaphores)]() mutable {
			auto acquire_batch = ste_device_queue::thread_allocate_batch();
			auto& command_buffer = acquire_batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();

				auto barrier = pipeline_barrier(src_stage,
												dst_stage,
												image_layout_transform_barrier(image,
																			   src_layout,
																			   dst_layout,
																			   src_access,
																			   dst_access));
				recorder << cmd_pipeline_barrier(barrier);
			}

			acquire_batch->signal_semaphores = std::move(signal_semaphores);
			acquire_batch->wait_semaphores = std::move(wait_semaphores);

			ste_device_queue::submit_batch(std::move(acquire_batch));
		});

		return future;
	}

	// Get a semaphore and boundary
	const auto release_acquire_boundary = lib::allocate_shared<boundary<void>>();
	auto sem = ctx.device().get_sync_primitives_pools().semaphores().claim();

	src_queue.enqueue([=, semptr = &sem.get(), &image, wait_semaphores = std::move(wait_semaphores)]() mutable {
		auto release_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Transform layout and release queue ownership
			auto barrier = pipeline_barrier(src_stage,
											pipeline_stage::top_of_pipe,
											queue_release_acquire_barrier(image,
																		  src_access,
																		  src_family,
																		  src_layout,
																		  access_flags::none,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		release_batch->signal_semaphores.emplace_back(semptr);
		release_batch->wait_semaphores = std::move(wait_semaphores);

		ste_device_queue::submit_batch(std::move(release_batch));
		release_acquire_boundary->signal();
	});
	auto dst_future = dst_queue.enqueue([=, sem = std::move(sem), &image, signal_semaphores = std::move(signal_semaphores)]() mutable {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Acquire queue ownership
			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											dst_stage,
											queue_release_acquire_barrier(image,
																		  access_flags::none,
																		  src_family,
																		  dst_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		release_acquire_boundary->get();

		acquire_batch->signal_semaphores = std::move(signal_semaphores);
		acquire_batch->wait_semaphores.emplace_back(std::move(sem), pipeline_stage::bottom_of_pipe);

		ste_device_queue::submit_batch(std::move(acquire_batch));
	});

	return dst_future;
}

/**
*	@brief	Enqueues image queue ownership transfer and image layout transform, perserving old content.
*			Deduces the access flags based on source and destination layouts. Layouts must no be undefined or preinitialized.
*
 *	@throws	ste_engine_exception		If image layout is undefined or preinitialized.
 *	
 *	@return	Enqueue future.
*/
auto inline queue_transfer(const ste_context &ctx,
						   const device_image_base &image,
						   ste_device_queue &src_queue,
						   ste_device_queue &dst_queue,
						   image_layout src_layout,
						   pipeline_stage src_stage,
						   image_layout dst_layout,
						   pipeline_stage dst_stage,
						   lib::vector<wait_semaphore> &&wait_semaphores = {},
						   lib::vector<const semaphore*> &&signal_semaphores = {}) {
	return queue_transfer(ctx,
						  image,
						  src_queue,
						  dst_queue,
						  src_layout, src_stage, access_flags_for_image_layout(src_layout),
						  dst_layout, dst_stage, access_flags_for_image_layout(dst_layout),
						  std::move(wait_semaphores), 
						  std::move(signal_semaphores));
}

/**
 *	@brief	Enqueues image queue ownership transfer and image layout transform, discarding old content.
 *	
 *	@return	Enqueue future.
 */
auto inline queue_transfer_discard(const ste_context &ctx,
								   const device_image_base &image,
								   const ste_queue_selector<ste_queue_selector_policy_strict> &dst_queue_selector,
								   pipeline_stage stage,
								   image_layout src_layout,
								   image_layout dst_layout,
								   access_flags dst_access,
								   lib::vector<wait_semaphore> &&wait_semaphores = {},
								   lib::vector<const semaphore*> &&signal_semaphores = {}) {
	auto &dst_queue = ctx.device().select_queue(dst_queue_selector);
	const ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	auto future = dst_queue.enqueue([=, &image, wait_semaphores = std::move(wait_semaphores), signal_semaphores = std::move(signal_semaphores)]() mutable {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											stage,
											queue_release_acquire_barrier(image,
																		  access_flags::none,
																		  0,
																		  src_layout,
																		  dst_access,
																		  dst_family,
																		  dst_layout));
			recorder << cmd_pipeline_barrier(barrier);
		}

		acquire_batch->wait_semaphores = std::move(wait_semaphores);
		acquire_batch->signal_semaphores = std::move(signal_semaphores);

		// Wait for release command to be submitted
		ste_device_queue::submit_batch(std::move(acquire_batch));
	});

	return future;
}

/**
 *	@brief	Enqueues image queue ownership transfer and image layout transform, discarding old content.
*			Deduces the access flags based on source and destination layouts. Layouts must no be undefined or preinitialized.
*			
 *	@throws	ste_engine_exception		If image layout is undefined or preinitialized.
 *	
 *	@return	Enqueue future.
*/
auto inline queue_transfer_discard(const ste_context &ctx,
								   const device_image_base &image,
								   const ste_queue_selector<ste_queue_selector_policy_strict> &dst_queue_selector,
								   pipeline_stage stage,
								   image_layout src_layout,
								   image_layout dst_layout,
								   lib::vector<wait_semaphore> &&wait_semaphores = {},
								   lib::vector<const semaphore*> &&signal_semaphores = {}) {
	return queue_transfer_discard(ctx,
								  image,
								  dst_queue_selector,
								  stage,
								  src_layout, 
								  dst_layout, 
								  access_flags_for_image_layout(dst_layout),
								  std::move(wait_semaphores), 
								  std::move(signal_semaphores));
}

}
}
