//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_family.hpp>
#include <device_buffer.hpp>

#include <pipeline_stage.hpp>
#include <access_flags.hpp>

#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <buffer_memory_barrier.hpp>

namespace ste {
namespace gl {

auto inline queue_release_acquire_barrier(const device_buffer_base &buffer,
										  access_flags src_access,
										  const ste_queue_family &src_family,
										  access_flags dst_access,
										  const ste_queue_family &dst_family) {
	return buffer_memory_barrier(buffer,
								 src_access,
								 dst_access,
								 src_family,
								 dst_family);
}

/**
 *	@brief	Enqueues queue ownership transfer for a buffer.
 *	
 *	@return	Enqueue future.
 */
auto inline queue_transfer(const ste_context &ctx,
						   const device_buffer_base &buffer,
						   ste_device_queue& src_queue,
						   ste_device_queue& dst_queue,
						   pipeline_stage src_stage,
						   access_flags src_access,
						   pipeline_stage dst_stage,
						   access_flags dst_access,
						   lib::vector<wait_semaphore> &&wait_semaphores = {},
						   lib::vector<semaphore*> &&signal_semaphores = {}) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	const ste_queue_family src_family = src_queue.queue_descriptor().family;
	const ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	// Tranfer only if needed
	if (src_family == dst_family) {
		return std::async(std::launch::deferred, []() {});
	}

	// Get a semaphore and boundary
	auto sem = ctx.device().get_sync_primitives_pools().semaphores().claim();

	src_queue.enqueue([=, semptr = &sem.get(), &buffer, wait_semaphores = std::move(wait_semaphores)]() mutable {
		auto release_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Release queue ownership
			auto barrier = pipeline_barrier(src_stage,
											pipeline_stage::top_of_pipe,
											queue_release_acquire_barrier(buffer,
																		  src_access,
																		  src_family,
																		  access_flags::none,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		release_batch->signal_semaphores.emplace_back(sem);
		release_batch->wait_semaphores = std::move(wait_semaphores);

		ste_device_queue::submit_batch(std::move(release_batch));
	});
	auto dst_future = dst_queue.enqueue([=, sem = std::move(sem), &buffer, signal_semaphores = std::move(signal_semaphores)]() mutable {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Acquire queue ownership
			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											dst_stage,
											queue_release_acquire_barrier(buffer,
																		  access_flags::none,
																		  src_family,
																		  dst_access,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be completed
		acquire_batch->signal_semaphores = std::move(signal_semaphores);
		acquire_batch->wait_sempahores.emplace_back(std::move(sem), pipeline_stage::bottom_of_pipe);

		ste_device_queue::submit_batch(std::move(acquire_batch));
	});

	return dst_future;
}

/**
*	@brief	Enqueues queue ownership transfer for a buffer, discarding old content.
*
*	@return	Enqueue future.
*/
auto inline queue_transfer_discard(const ste_context &ctx,
								   const device_buffer_base &buffer,
								   const ste_queue_selector<ste_queue_selector_policy_strict> &dst_queue_selector,
								   pipeline_stage stage,
								   access_flags dst_access,
								   lib::vector<wait_semaphore> &&wait_semaphores = {},
								   lib::vector<semaphore*> &&signal_semaphores = {}) {
	auto &dst_queue = ctx.device().select_queue(dst_queue_selector);
	const ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	auto future = dst_queue.enqueue([=, &buffer]() mutable {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											stage,
											queue_release_acquire_barrier(buffer,
																		  access_flags::none,
																		  0,
																		  dst_access,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		acquire_batch->signal_semaphores = std::move(signal_semaphores);
		acquire_batch->wait_semaphores = std::move(wait_semaphores);

		// Wait for release command to be submitted
		ste_device_queue::submit_batch(std::move(acquire_batch));
	});

	return future;
}

}
}
