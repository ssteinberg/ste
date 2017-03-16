//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <fence.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <device_resource_allocation_policy.hpp>

#include <vk_command_recorder.hpp>
#include <vk_cmd_copy_buffer.hpp>

#include <vector>

namespace StE {
namespace GL {

template <typename T, class policy>
void copy_initial_data(const ste_context &ctx,
					   device_buffer<T, policy> &buffer,
					   const std::vector<T> &data) {
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible_coherent>;

	auto copy_count = std::min<std::uint64_t>(data.size(), buffer.get().get_elements_count());

	staging_buffer_t staging_buffer(ctx, copy_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		memcpy(ptr->get_mapped_ptr(), data.data(), copy_count * sizeof(T));
	}

	// Select queue
	auto queue_type = ste_queue_type::data_transfer_queue;
	auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);

	// Create a batch
	auto batch = ctx.device().select_queue(queue_selector)->allocate_batch();
	auto& command_buffer = batch.acquire_command_buffer();
	auto fence = batch.get_fence();

	// Enqueue on a transfer queue
	ctx.device().enqueue(queue_selector, [&]() {
		// Record and submit a one-time batch
		{
			auto recorder = command_buffer.record();
			// Copy to live buffer
			recorder << vk_cmd_copy_buffer(staging_buffer.get(), buffer.get());
		}

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for completion
	(**fence).get();
}

template <typename T, int atom_size, class policy>
void copy_initial_data(const ste_context &ctx,
					   device_buffer_sparse<T, atom_size, policy> &buffer,
					   const std::vector<T> &data) {
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible_coherent>;

	auto copy_count = data.size();

	staging_buffer_t staging_buffer(ctx, copy_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	{
		// Copy to staging
		auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
		memcpy(ptr->get_mapped_ptr(), data.data(), copy_count * sizeof(T));
	}

	// Select queue
	auto queue_type = ste_queue_type::data_transfer_sparse_queue;
	auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);

	// Create a batch
	auto batch = ctx.device().select_queue(queue_selector)->allocate_batch();
	auto& command_buffer = batch.acquire_command_buffer();
	auto fence = batch.get_fence();

	// Enqueue on a transfer queue
	ctx.device().enqueue(queue_selector, [&]() {
		// Bind sparse memory
		typename device_buffer_sparse<T, atom_size, policy>::bind_range_t bind = { 0, copy_count };
		buffer.cmd_bind_sparse_memory(ste_device_queue::thread_queue(), {}, { bind }, {}, {});

		{
			// Record and submit a one-time batch
			auto recorder = command_buffer.record();

			// Copy to live buffer
			VkBufferCopy copy = { 0, 0, copy_count * sizeof(T) };
			recorder << vk_cmd_copy_buffer(staging_buffer.get(), buffer.get(), { copy });
		}

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Wait for completion
	(**fence).get();
}

}
}
