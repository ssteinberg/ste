//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <buffer_usage.hpp>
#include <unique_fence.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <command_recorder.hpp>
#include <cmd_copy_buffer.hpp>

#include <lib/vector.hpp>
#include <cstring>

#include <ste_engine_exceptions.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename T, class policy>
void copy_data_buffer(const ste_context &ctx,
					  device_buffer<T, policy> &buffer,
					  const lib::vector<T> &data,
					  std::size_t offset = 0) {
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible_coherent>;

	// Don't allow copying pass buffer end
	if (data.size() > buffer.get().get_elements_count() + offset) {
		throw ste_engine_exception("Buffer overflow");
	}

	auto copy_count = data.size();
	
	// Select queue
	auto queue_type = ste_queue_type::data_transfer_queue;
	auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
	auto& q = ctx.device().select_queue(queue_selector);

	// Staging buffer
	staging_buffer_t staging_buffer(ctx,
									copy_count,
									buffer_usage::transfer_src);
	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		std::memcpy(ptr->get_mapped_ptr(), data.data(), static_cast<std::size_t>(copy_count * sizeof(T)));
	}

	// Create a batch
	auto batch = q.allocate_batch();
	auto& command_buffer = batch->acquire_command_buffer();
	auto fence = batch->get_fence_ptr();

	// Enqueue on a transfer queue
	ctx.device().enqueue(queue_selector, [&]() {
		// Record and submit a one-time batch
		{
			auto recorder = command_buffer.record();
			// Copy to live buffer
			VkBufferCopy range = { 0, offset * sizeof(T), copy_count * sizeof(T) };
			recorder << cmd_copy_buffer(staging_buffer, buffer, { range });
		}

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for completion
	(*fence)->get_wait();
}

template <typename T, int atom_size, class policy>
void copy_data_buffer(const ste_context &ctx,
					  device_buffer_sparse<T, atom_size, policy> &buffer,
					  const lib::vector<T> &data,
					  std::uint64_t offset = 0) {
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible_coherent>;

	auto copy_count = data.size();

	// Select queue
	auto queue_type = ste_queue_type::data_transfer_sparse_queue;
	auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
	auto& q = ctx.device().select_queue(queue_selector);

	// Staging buffer
	staging_buffer_t staging_buffer(ctx,
									copy_count,
									buffer_usage::transfer_src);
	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		std::memcpy(ptr->get_mapped_ptr(), data.data(), copy_count * sizeof(T));
	}

	// Create a batch
	auto batch = q.allocate_batch();
	auto& command_buffer = batch->acquire_command_buffer();
	auto fence = batch->get_fence_ptr();

	// Enqueue on a transfer queue
	ctx.device().enqueue(queue_selector, [&]() {
		// Bind sparse memory
		typename device_buffer_sparse<T, atom_size, policy>::bind_range_t bind = { offset, copy_count };

		{
			// Record and submit a one-time batch
			auto recorder = command_buffer.record();

			// Copy to live buffer
			VkBufferCopy copy = { 0, offset * sizeof(T), copy_count * sizeof(T) };
			recorder << buffer.cmd_bind_sparse_memory({}, { bind }, {}, {});
			recorder << cmd_copy_buffer(staging_buffer, buffer, { copy });
		}

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for completion
	(*fence)->get_wait();
}

}

}
}
