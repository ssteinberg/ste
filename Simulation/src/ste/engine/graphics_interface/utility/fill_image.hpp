//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <job.hpp>

#include <surface.hpp>
#include <surface_type_traits.hpp>
#include <surface_utilities.hpp>

#include <device_image.hpp>
#include <device_image_queue_transfer.hpp>
#include <device_image_exceptions.hpp>

#include <device_buffer.hpp>

#include <pipeline_barrier.hpp>
#include <image_memory_barrier.hpp>
#include <buffer_memory_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_copy_buffer_to_image.hpp>

#include <format_rtti.hpp>

#include <cstring>
#include <limits>

namespace ste {
namespace gl {

namespace _internal {

template <int dimensions, class allocation_policy, typename selector_policy, typename Surface>
auto fill_image_array(const device_image<dimensions, allocation_policy> &image,
					  Surface &&surface,
					  image_layout final_layout,
					  ste_queue_selector<selector_policy> &&final_queue_selector,
					  std::uint32_t initial_layer,
					  std::uint32_t initial_level,
					  std::uint32_t max_layer,
					  std::uint32_t max_level,
					  const lib::vector<wait_semaphore> &wait_semaphores = {},
					  const lib::vector<const semaphore*> &signal_semaphores = {}) {
	static_assert(resource::is_surface_v<Surface>);

	using block_type = typename Surface::block_type;

	static constexpr gl::format format = Surface::surface_format();
	static constexpr gl::image_type image_type = Surface::surface_image_type();
	using extent_type = gl::image_extent_type_t<gl::image_dimensions_v<image_type>>;

	static constexpr auto image_dimensions = gl::image_dimensions_v<image_type>;
	auto image_format = image.get_format();

	static_assert(image_dimensions == dimensions);

	// Validate formats
	if (image_format != format) {
		throw device_image_format_exception("Surface and image format mismatch");
	}

	auto future = image.parent_context().engine().task_scheduler().schedule_now([=, &image, surface = std::move(surface), final_queue_selector = std::move(final_queue_selector)]() {
		const ste_context &ctx = image.parent_context();

		const auto surface_block_bytes = surface.block_bytes();
		const extent_type image_extent = resource::surface_utilities::extent(image.get_extent(), initial_level);
		const extent_type surface_extent = surface.extent();
		const auto aspect = static_cast<VkImageAspectFlags>(format_aspect(image_format));
		const auto bytes = surface.bytes();

		// Calculate layers and levels to copy
		auto layers = std::min(static_cast<std::uint32_t>(surface.layers()),
							   image.get_layers());
		layers = std::min(layers, max_layer + 1);
		auto mips = std::min(static_cast<std::uint32_t>(surface.levels()),
							 image.get_mips());
		mips = std::min(mips, max_level + 1);

		// Validate size
		if (surface_extent != image_extent) {
			throw device_image_format_exception("Surface and image extent mismatch. Surface extent at level 0 must equal to image extent at level initial_level.");
		}

		// Create staging buffer
		using staging_buffer_t = device_buffer<block_type, device_resource_allocation_policy_host_visible>;
		const auto blocks = surface.blocks_layer() * layers;
		staging_buffer_t staging_buffer(ctx,
										blocks,
										gl::buffer_usage::transfer_src,
										"fill_image staging buffer");

		// Copy to destination image
		{
			auto mmap_blocks_ptr = staging_buffer.get_underlying_memory().template mmap<block_type>(0, blocks);
			std::memcpy(mmap_blocks_ptr->get_mapped_ptr(),
						surface.data(),
						bytes);
			// Flush written memory
			mmap_blocks_ptr->flush_ranges({ vk::vk_mapped_memory_range{ 0, blocks } });
		}

		// Create regions to copy
		lib::vector<VkBufferImageCopy> regions;
		regions.reserve((layers - initial_layer) * (mips - initial_level));
		for (std::uint32_t l = initial_layer; l < layers; ++l) {
			for (std::uint32_t m = initial_level; m < mips; ++m) {
				auto extent = surface.extent(m);
				auto buffer_offset = surface.offset_blocks(l, m);

				VkBufferImageCopy copy_region = {
					buffer_offset, 0, 0,
					{ aspect, m, l, 1 },
					{ 0, 0, 0 },
					{ 1, 1, 1 }
				};
				if constexpr (dimensions > 0) copy_region.imageExtent.width = static_cast<std::uint32_t>(extent[0]);
				if constexpr (dimensions > 1) copy_region.imageExtent.height = static_cast<std::uint32_t>(extent[1]);
				if constexpr (dimensions > 2) copy_region.imageExtent.depth = static_cast<std::uint32_t>(extent[2]);

				regions.push_back(copy_region);
			}
		}

		// Select queue
		auto queue_type = ste_queue_type::data_transfer_queue;
		auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
		auto &q = ctx.device().select_queue(queue_selector);

		// Create a batch
		auto batch = q.allocate_batch<staging_buffer_t>(std::move(staging_buffer));
		auto semaphore = ctx.device().get_sync_primitives_pools().semaphores().claim();

		// Enqueue mipmap copy on a transfer queue
		auto copy_to_image_future = q.enqueue([&]() {
			// Record and submit a one-time batch
			auto &command_buffer = batch->acquire_command_buffer();

			const auto &dst_buffer = batch->user_data();
			{
				auto recorder = command_buffer.record();

				// Move to transfer layouts
				auto barrier = pipeline_barrier(pipeline_stage::top_of_pipe | pipeline_stage::host,
												pipeline_stage::transfer,
												image_memory_barrier(image,
																	 image_layout::undefined,
																	 image_layout::transfer_dst_optimal,
																	 access_flags::none,
																	 access_flags::transfer_write),
												buffer_memory_barrier(dst_buffer,
																	  access_flags::host_write,
																	  access_flags::transfer_read));
				recorder << cmd_pipeline_barrier(barrier);

				// Copy to image
				recorder << cmd_copy_buffer_to_image(dst_buffer,
													 image,
													 image_layout::transfer_dst_optimal,
													 regions);
			}

			ste_device_queue::submit_batch(std::move(batch), wait_semaphores, { &semaphore.get() });
		});
		copy_to_image_future.get();

		// Transfer ownership
		pipeline_stage pipeline_stages_for_final_layout = all_possible_pipeline_stages_for_access_flags(access_flags_for_image_layout(final_layout));
		auto queue_transfer_future = queue_transfer(ctx,
													image,
													q,
													ctx.device().select_queue(final_queue_selector),
													image_layout::transfer_dst_optimal,
													pipeline_stage::transfer,
													final_layout,
													pipeline_stages_for_final_layout,
													{ wait_semaphore(&semaphore.get(), pipeline_stage::bottom_of_pipe) },
													signal_semaphores);

		// Wait for completion
		queue_transfer_future.get();
	});

	return make_job(std::move(future));
}

}

/**
*	@brief	Copies image from surface into target 1d image.
*
*	@param	image				Target image
*	@param	surface				1d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<1, allocation_policy> &image,
				resource::surface_1d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_level = 0,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0,
										 initial_level,
										 std::numeric_limits<std::uint32_t>::max() - 1,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface into target 1d array image.
*
*	@param	image				Target image
*	@param	surface				1d array surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<1, allocation_policy> &image,
				resource::surface_1d_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_layer = 0,
				std::uint32_t initial_level = 0,
				std::uint32_t max_layer = std::numeric_limits<std::uint32_t>::max() - 1,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface into target 2d image.
*
*	@param	image				Target image
*	@param	surface				2d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_2d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_level = 0,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0,
										 initial_level,
										 std::numeric_limits<std::uint32_t>::max() - 1,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface into target 2d array image.
*
*	@param	image				Target image
*	@param	surface				2d array surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_2d_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_layer = 0,
				std::uint32_t initial_level = 0,
				std::uint32_t max_layer = std::numeric_limits<std::uint32_t>::max() - 1,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface into target 3d image.
*
*	@param	image				Target image
*	@param	surface				3d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<3, allocation_policy> &image,
				resource::surface_3d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_level = 0,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0,
										 initial_level,
										 std::numeric_limits<std::uint32_t>::max() - 1,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface cubemap into target 2d array image.
*
*	@param	image				Target image
*	@param	surface				Cubemap surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_cubemap<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_layer = 0,
				std::uint32_t initial_level = 0,
				std::uint32_t max_layer = std::numeric_limits<std::uint32_t>::max() - 1,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface cubemap into target 2d array image.
*
*	@param	image				Target image
*	@param	surface				Array cubemap surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_cubemap_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				std::uint32_t initial_layer = 0,
				std::uint32_t initial_level = 0,
				std::uint32_t max_layer = std::numeric_limits<std::uint32_t>::max() - 1,
				std::uint32_t max_level = std::numeric_limits<std::uint32_t>::max() - 1,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 wait_semaphores,
										 signal_semaphores);
}

/**
*	@brief	Copies image from surface into target image.
*
*	@param	image				Target image
*	@param	surface				Surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <class allocation_policy, typename selector_policy, int dimensions, typename Surface>
auto fill_image(const device_image<dimensions, allocation_policy> &image,
				Surface &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector,
				const lib::vector<wait_semaphore> &wait_semaphores = {},
				const lib::vector<const semaphore*> &signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0,
										 0,
										 std::numeric_limits<std::uint32_t>::max() - 1,
										 std::numeric_limits<std::uint32_t>::max() - 1,
										 wait_semaphores,
										 signal_semaphores);
}

}
}
