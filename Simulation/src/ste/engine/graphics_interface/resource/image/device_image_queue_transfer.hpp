//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image.hpp>
#include <device_resource_queue_ownership.hpp>

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
								   VkImageLayout dst_layout) {
	return GL::vk_pipeline_barrier(GL::vk_image_memory_barrier(image.get(),
															   src_layout,
															   dst_layout,
															   src_access,
															   dst_access,
															   src_family,
															   dst_family));
}

template <int dimensions, class allocation_policy>
void queue_transfer(device_image<dimensions, allocation_policy> &image,
					const ste_device_queue::queue_index_t &dst_queue_index,
					VkAccessFlags src_access,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout) {
	auto& ctx = image.ctx;
	auto src_q_idx = image.queue_ownership.queue_index;
	
	// Tranfer only if needed
	if (src_q_idx == dst_queue_index) {
		// TODO: Make this a layout change
		assert(false);
		return;
	}

	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;

	auto src_layout = image.image_layout.layout();

	auto& src_q = ctx.device().select_queue(src_q_idx);
	auto& dst_q = ctx.device().select_queue(dst_queue_index);

	auto src_family = src_q->queue_descriptor().family;
	auto dst_family = dst_q->queue_descriptor().family;

	// Get a semaphore
	auto semaphore = ctx.device().get_sync_primitives_pools().semaphores().claim();

	struct user_data_t {
		semaphore_t semaphore;
		std::mutex m;

		user_data_t(semaphore_t &&semaphore) : semaphore(std::move(semaphore)) {}
		user_data_t(user_data_t&&o) noexcept : semaphore(std::move(o.semaphore)) {}
	};
	auto user_data = std::make_shared<user_data_t>(user_data_t{ std::move(semaphore) });

	src_q->enqueue([=, &image]() {
		auto release_batch = ste_device_queue::thread_allocate_batch<std::shared_ptr<user_data_t>>(user_data);
		auto& command_buffer = release_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = queue_release_acquire_barrier(image,
														 src_access,
														 src_family,
														 src_layout,
														 dst_access,
														 dst_family,
														 src_layout);
			recorder << GL::vk_cmd_pipeline_barrier(barrier);
		}

		std::unique_lock<std::mutex> l(release_batch->user_data()->m);
		ste_device_queue::submit_batch(std::move(release_batch), {}, { &*release_batch->user_data()->semaphore });
	});
	dst_q->enqueue([=, &image]() {
		auto acquire_batch = ste_device_queue::thread_allocate_batch<std::shared_ptr<user_data_t>>(user_data);
		auto& command_buffer = acquire_batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			auto barrier = queue_release_acquire_barrier(image,
														 src_access,
														 src_family,
														 src_layout,
														 dst_access,
														 dst_family,
														 dst_layout);
			recorder << GL::vk_cmd_pipeline_barrier(barrier);
		}

		std::unique_lock<std::mutex> l(acquire_batch->user_data()->m);
		ste_device_queue::submit_batch(std::move(acquire_batch), { std::make_pair(&*acquire_batch->user_data()->semaphore, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) }, {});
	});

	image.queue_ownership.queue_index = dst_queue_index;
	image.image_layout.image_layout = dst_layout;
}

template <int dimensions, class allocation_policy>
void queue_transfer(device_image<dimensions, allocation_policy> &image,
					const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector,
					VkAccessFlags src_access,
					VkAccessFlags dst_access,
					VkImageLayout dst_layout) {
	auto& ctx = image.ctx;

	auto& dst_q = ctx.device().select_queue(queue_selector);
	auto dst_q_idx = dst_q->index();

	queue_transfer(image,
				   dst_q_idx,
				   src_access,
				   dst_access,
				   dst_layout);
}

template <int dimensions, class allocation_policy>
void queue_transfer_discard(device_image<dimensions, allocation_policy> &image,
							const ste_queue_selector<ste_queue_selector_default_policy> &queue_selector) {
	auto& dst_q = image.ctx.device().select_queue(queue_selector);
	auto dst_q_idx = dst_q->index();

	image.queue_ownership.queue_index = dst_q_idx;
}

template <int dimensions, class allocation_policy>
void queue_transfer_discard(device_image<dimensions, allocation_policy> &image,
							const ste_device_queue::queue_index_t &dst_queue_index) {
	image.queue_ownership.queue_index = dst_queue_index;
}

}
}
