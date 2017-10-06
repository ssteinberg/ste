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
					  layers_t initial_layer,
					  levels_t initial_level,
					  layers_t max_layer,
					  levels_t max_level,
					  lib::vector<wait_semaphore> &&wait_semaphores = {},
					  lib::vector<semaphore*> &&signal_semaphores = {}) {
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

	auto future = image.parent_context().engine().task_scheduler().schedule_now(
		[=, &image, surface = std::move(surface), final_queue_selector = std::move(final_queue_selector), wait_semaphores = std::move(wait_semaphores), signal_semaphores = std::move(signal_semaphores)]() mutable {
		const ste_context &ctx = image.parent_context();

		const extent_type image_extent = resource::surface_utilities::extent(image.get_extent(), initial_level);
		const extent_type surface_extent = surface.extent();

		// Calculate layers and levels to copy
		const auto last_layer = std::min(std::min(surface.layers(), image.get_layers()) - 1_layer,
										 max_layer);
		const auto last_mip = std::min(std::min(surface.levels(), image.get_mips())  - 1_mip,
									   max_level);
		const auto layers = last_layer - initial_layer + 1_layer;
		const auto mips = last_mip - initial_level + 1_mip;

		// Validate size
		if (surface_extent != image_extent) {
			throw device_image_format_exception("Surface and image extent mismatch. Surface extent at level 0 must equal to image extent at level initial_level.");
		}

		const auto block_bytes = resource::surface_utilities::block_bytes<format>();

		// Create staging buffer
		using staging_buffer_t = device_buffer<block_type, device_resource_allocation_policy_host_visible>;
		const auto blocks = surface.blocks_layer() * static_cast<std::uint64_t>(layers);
		staging_buffer_t staging_buffer(ctx,
										blocks,
										gl::buffer_usage::transfer_src,
										"fill_image staging buffer");

		// Copy to destination image
		{
			auto mmap_blocks_ptr = staging_buffer.get_underlying_memory().template mmap<block_type>(0, blocks);
			std::memcpy(mmap_blocks_ptr->get_mapped_ptr(),
						surface.data(),
						blocks * static_cast<std::size_t>(block_bytes));
			// Flush written memory
			mmap_blocks_ptr->flush_ranges({ vk::vk_mapped_memory_range{ 0, blocks } });
		}

		// Create regions to copy
		lib::vector<buffer_image_copy_region_t> regions;
		regions.reserve(static_cast<std::size_t>(layers) * static_cast<std::size_t>(mips));
		for (auto l = initial_layer; l <= last_layer; ++l) {
			for (auto m = initial_level; m <= last_mip; ++m) {
				auto extent = surface.extent(m);
				auto buffer_offset_blocks = resource::surface_utilities::offset_blocks<format>(surface_extent,
																							   mips,
																							   l - initial_layer,
																							   m - initial_level);

				buffer_image_copy_region_t copy_region;
				copy_region.buffer_offset = buffer_offset_blocks;
				copy_region.image_format = image_format;
				copy_region.mip = m;
				copy_region.base_layer = l;
				copy_region.layers = 1_layer;
				if constexpr (dimensions > 0) copy_region.extent.x = static_cast<std::uint32_t>(extent[0]);
				if constexpr (dimensions > 1) copy_region.extent.y = static_cast<std::uint32_t>(extent[1]);
				if constexpr (dimensions > 2) copy_region.extent.z = static_cast<std::uint32_t>(extent[2]);

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
		auto copy_to_image_future = q.enqueue([&, semptr = &semaphore.get()]() {
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

			batch->wait_semaphores = std::move(wait_semaphores);
			batch->signal_semaphores.emplace_back(semptr);

			ste_device_queue::submit_batch(std::move(batch));
		});
		copy_to_image_future.get();

		// Transfer ownership
		pipeline_stage pipeline_stages_for_final_layout = all_possible_pipeline_stages_for_access_flags(access_flags_for_image_layout(final_layout));
		lib::vector<wait_semaphore> queue_transfer_wait_semaphores;
		queue_transfer_wait_semaphores.emplace_back(std::move(semaphore), pipeline_stage::bottom_of_pipe);
		auto queue_transfer_future = queue_transfer(ctx,
													image,
													q,
													ctx.device().select_queue(final_queue_selector),
													image_layout::transfer_dst_optimal,
													pipeline_stage::transfer,
													final_layout,
													pipeline_stages_for_final_layout,
													std::move(queue_transfer_wait_semaphores),
													std::move(signal_semaphores));

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
				levels_t initial_level = 0_mip,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0_layer,
										 initial_level,
										 all_layers,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				layers_t initial_layer = 0_layer,
				levels_t initial_level = 0_mip,
				layers_t max_layer = all_layers,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				levels_t initial_level = 0_mip,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0_layer,
										 initial_level,
										 all_layers,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				layers_t initial_layer = 0_layer,
				levels_t initial_level = 0_mip,
				layers_t max_layer = all_layers,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				levels_t initial_level = 0_mip,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0_layer,
										 initial_level,
										 all_layers,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				layers_t initial_layer = 0_layer,
				levels_t initial_level = 0_mip,
				layers_t max_layer = all_layers,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				layers_t initial_layer = 0_layer,
				levels_t initial_level = 0_mip,
				layers_t max_layer = all_layers,
				levels_t max_level = all_mips,
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 initial_layer,
										 initial_level,
										 max_layer,
										 max_level,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
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
				lib::vector<wait_semaphore> &&wait_semaphores = {},
				lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector),
										 0_layer,
										 0_mip,
										 all_layers,
										 all_mips,
										 std::move(wait_semaphores),
										 std::move(signal_semaphores));
}

}
}
