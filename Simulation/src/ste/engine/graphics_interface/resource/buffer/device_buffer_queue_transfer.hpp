//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_family.hpp>
#include <device_buffer.hpp>
#include <device_resource_queue_ownership.hpp>

#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <buffer_memory_barrier.hpp>

#include <boundary.hpp>
#include <memory>

namespace StE {
namespace GL {

auto inline queue_release_acquire_barrier(const device_buffer_base &buffer,
										  VkAccessFlags src_access,
										  const ste_queue_family &src_family,
										  VkAccessFlags dst_access,
										  const ste_queue_family &dst_family) {
	return buffer_memory_barrier(buffer,
								 src_access,
								 dst_access,
								 src_family,
								 dst_family);
}

void inline queue_transfer(const ste_context &ctx,
						   device_buffer_base &buffer,
						   const ste_queue_family &dst_family,
						   VkPipelineStageFlags src_stage,
						   VkAccessFlags src_access,
						   VkPipelineStageFlags dst_stage,
						   VkAccessFlags dst_access) {
	assert(!ste_device_queue::is_queue_thread() && "Should not be called from a queue");

	auto src_family = buffer.owner_queue_family();

	auto& src_q = ctx.device().select_queue(make_family_queue_selector(src_family));
	auto& dst_q = ctx.device().select_queue(make_family_queue_selector(dst_family));

	// Tranfer only if needed
	if (src_family == dst_family) {
		return;
	}

	// Get a semaphore and boundary
	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;
	using user_data_t = std::shared_ptr<semaphore_t>;
	auto user_data = std::make_shared<semaphore_t>(ctx.device().get_sync_primitives_pools().semaphores().claim());
	auto release_acquire_boundary = std::make_shared<boundary<void>>();

	src_q->enqueue([=, &buffer]() {
		auto release_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(src_stage,
											VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
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
	dst_q->enqueue([=, &buffer]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<user_data_t>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = pipeline_barrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
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
		ste_device_queue::submit_batch(std::move(acquire_batch),
		{ std::make_pair(static_cast<VkSemaphore>(*acquire_batch->user_data()), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) }, {});
	});

	buffer.queue_ownership.family.store(dst_family, std::memory_order_release);
}

template <typename selector_policy>
void queue_transfer(const ste_context &ctx,
					device_buffer_base &buffer,
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
				   buffer,
				   dst_family,
				   src_stage,
				   src_access,
				   dst_stage,
				   dst_access);
}

}
}
