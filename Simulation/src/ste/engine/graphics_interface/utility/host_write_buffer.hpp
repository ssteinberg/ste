//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_queue_type.hpp>

#include <buffer_usage.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <command_recorder.hpp>
#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_copy_buffer.hpp>

#include <cstring>

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Container>
void host_write_buffer(const ste_context &ctx,
					   Container &gl_container,
					   const typename Container::value_type* data,
					   std::size_t size,
					   std::size_t offset = 0) {
	using T = typename Container::value_type;
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible>;

	// Don't allow copying pass buffer end
	if (size > gl_container.get().get_elements_count() + offset) {
		throw ste_engine_exception("Buffer overflow");
	}

	auto copy_count = size;

	// Select queue
	auto queue_type = ste_queue_type::data_transfer_queue;
	auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);

	// Staging buffer
	staging_buffer_t staging_buffer(ctx,
									copy_count,
									buffer_usage::transfer_src,
									"host_write_buffer staging buffer");

	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		std::memcpy(ptr->get_mapped_ptr(), data, static_cast<std::size_t>(copy_count * sizeof(T)));
		// Flush written memory
		ptr->flush_ranges({ vk::vk_mapped_memory_range{ 0, copy_count } });
	}

	// Enqueue on a transfer queue
	auto f = ctx.device().enqueue(queue_selector, [&, staging = std::move(staging_buffer)]() mutable {
		// Create a batch
		auto batch = ste_device_queue::thread_allocate_batch<staging_buffer_t>(std::move(staging));
		auto& command_buffer = batch->acquire_command_buffer();

		buffer_copy_region_t range = { 0, offset, copy_count * byte_t(sizeof(T)) };

		// Record and submit a one-time batch
		{
			auto recorder = command_buffer.record();

			recorder << cmd_pipeline_barrier(pipeline_barrier(pipeline_stage::host,
															  pipeline_stage::transfer,
															  buffer_memory_barrier(batch->user_data(),
																					access_flags::host_write,
																					access_flags::transfer_read)));

			// Copy to live buffer
			recorder << cmd_copy_buffer(batch->user_data(),
										gl_container,
										{ range });
		}
		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for submission
	f.get();
}

template <typename Container, typename = typename std::enable_if<Container::sparse_container>::type>
void host_write_buffer_and_resize(const ste_context &ctx,
								  Container &gl_container,
								  const typename Container::value_type* data,
								  std::size_t size,
								  std::size_t offset = 0) {
	using T = typename Container::value_type;
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible>;

	// Don't allow copying pass buffer end
	if (size > gl_container.get().get_elements_count() + offset) {
		throw ste_engine_exception("Buffer overflow");
	}

	const auto copy_count = size;

	// Select queue (might need to sparse bind)
	const auto queue_type = ste_queue_type::data_transfer_sparse_queue;
	const auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);

	// Staging buffer
	staging_buffer_t staging_buffer(ctx,
									copy_count,
									buffer_usage::transfer_src,
									"host_write_buffer_and_resize staging buffer");

	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		std::memcpy(ptr->get_mapped_ptr(), data, static_cast<std::size_t>(copy_count * sizeof(T)));
		// Flush written memory
		ptr->flush_ranges({ vk::vk_mapped_memory_range{ 0, copy_count } });
	}

	// Enqueue on a transfer queue
	auto f = ctx.device().enqueue(queue_selector, [=, &gl_container, staging = std::move(staging_buffer)]() {
		// Create a batch
		auto batch = ste_device_queue::thread_allocate_batch<staging_buffer_t>(std::move(staging));
		auto& command_buffer = batch->acquire_command_buffer();

		VkBufferCopy range = { 0, offset * sizeof(T), copy_count * sizeof(T) };

		// Record and submit a one-time batch
		{
			auto recorder = command_buffer.record();

			// Resize sparse buffer, if needed
			if (gl_container.size() < copy_count + offset)
				recorder << gl_container.resize_cmd(copy_count + offset);

			recorder << cmd_pipeline_barrier(pipeline_barrier(pipeline_stage::host,
															  pipeline_stage::transfer,
															  buffer_memory_barrier(batch->user_data(),
																					access_flags::host_write,
																					access_flags::transfer_read)));

			// Copy to live buffer
			recorder << cmd_copy_buffer(batch->user_data(),
										gl_container,
										{ range });
		}

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for submission
	f.get();
}

}

}
}
