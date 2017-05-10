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

#include <boundary.hpp>
#include <memory>

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
						   access_flags dst_access) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	ste_queue_family src_family = src_queue.queue_descriptor().family;
	ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	// Tranfer only if needed
	if (src_family == dst_family) {
		return std::async(std::launch::deferred, []() {});
	}

	// Get a semaphore and boundary
	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;
	using user_data_t = std::shared_ptr<semaphore_t>;
	auto user_data = std::make_shared<semaphore_t>(ctx.device().get_sync_primitives_pools().semaphores().claim());
	auto release_acquire_boundary = std::make_shared<boundary<void>>();

	auto src_future = src_queue.enqueue([=, &buffer]() {
		auto release_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Release queue ownership
			auto barrier = pipeline_barrier(src_stage,
											pipeline_stage::top_of_pipe,
											queue_release_acquire_barrier(buffer,
																		  src_access,
																		  src_family,
																		  dst_access,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		ste_device_queue::submit_batch(std::move(release_batch), {}, { *release_batch->user_data() });
		release_acquire_boundary->signal();
	});
	auto dst_future = dst_queue.enqueue([=, &buffer, f = std::move(src_future)]() {
		f.wait();

		auto acquire_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Acquire queue ownership
			auto barrier = pipeline_barrier(pipeline_stage::top_of_pipe,
											dst_stage,
											queue_release_acquire_barrier(buffer,
																		  src_access,
																		  src_family,
																		  dst_access,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		release_acquire_boundary->get();
		const semaphore &sem = *acquire_batch->user_data();
		ste_device_queue::submit_batch(std::move(acquire_batch), { wait_semaphore(&sem, pipeline_stage::bottom_of_pipe) }, {});
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
								   access_flags src_access,
								   access_flags dst_access) {
	auto &dst_queue = *ctx.device().select_queue(dst_queue_selector);
	ste_queue_family dst_family = dst_queue.queue_descriptor().family;

	auto future = dst_queue.enqueue([=, &buffer]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<>();
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(pipeline_stage::bottom_of_pipe,
											stage,
											queue_release_acquire_barrier(buffer,
																		  src_access,
																		  0,
																		  dst_access,
																		  dst_family));
			recorder << cmd_pipeline_barrier(barrier);
		}

		// Wait for release command to be submitted
		ste_device_queue::submit_batch(std::move(acquire_batch));
	});

	return future;
}

}
}
