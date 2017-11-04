//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_queue_type.hpp>

#include <device_image_exceptions.hpp>

#include <buffer_usage.hpp>
#include <device_buffer.hpp>
#include <image_layout.hpp>
#include <device_image.hpp>
#include <surface.hpp>
#include <surface_utilities.hpp>
#include <device_resource_allocation_policy.hpp>

#include <command_recorder.hpp>
#include <pipeline_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_copy_image_to_buffer.hpp>

#include <cstring>
#include <lib/vector.hpp>
#include <lib/unique_ptr.hpp>
#include <limits>

namespace ste {
namespace gl {

namespace _internal {

template <gl::format format, gl::image_type image_type, class allocation_policy>
auto host_read_image(const ste_context &ctx,
					 const device_image<image_dimensions_v<image_type>, allocation_policy> &image,
					 layers_t initial_layer,
					 levels_t initial_level,
					 layers_t max_layer,
					 levels_t max_level,
					 lib::vector<wait_semaphore> &&wait_semaphores = {},
					 lib::vector<semaphore*> &&signal_semaphores = {}) {
	using block_type = typename gl::format_traits<format>::block_type;
	using staging_buffer_t = device_buffer<block_type, device_resource_allocation_policy_host_visible>;
	using extent_type = glm::u32vec3;
	static constexpr int dimensions = image_dimensions_v<image_type>;

	// Validate format
	if (image.get_format() != format) {
		throw device_image_format_exception("Surface and image format mismatch");
	}

	// Calculate layers and levels to copy
	const auto last_layer = std::min(image.get_layers() - 1_layer,
									 max_layer);
	const auto last_mip = std::min(image.get_mips() - 1_mip,
								   max_level);
	const auto layers = last_layer - initial_layer + 1_layer;
	const auto mips = last_mip - initial_level + 1_mip;

	if (layers <= initial_layer) {
		throw device_image_format_exception("initial_layer out of bounds");
	}
	if (mips <= initial_level) {
		throw device_image_format_exception("initial_level out of bounds");
	}

	// Surface properties
	const auto block_bytes = resource::surface_utilities::block_bytes<format>();
	const auto surface_extent = resource::surface_utilities::extent(image.get_extent(), initial_level);
	const auto surface_bytes = resource::surface_utilities::bytes<format>(surface_extent,
																		  mips,
																		  layers);
	const auto surface_blocks = resource::surface_utilities::blocks_layer<format>(surface_extent, mips) * static_cast<std::uint64_t>(layers);

	// Create regions to copy
	lib::vector<buffer_image_copy_region_t> regions;
	regions.reserve(static_cast<std::size_t>(layers) * static_cast<std::size_t>(mips));
	for (auto l = initial_layer; l <= last_layer; ++l) {
		for (auto m = initial_level; m <= last_mip; ++m) {
			auto extent = resource::surface_utilities::extent(image.get_extent(), m);
			auto buffer_offset_blocks = resource::surface_utilities::offset_blocks<format>(surface_extent,
																						   mips,
																						   l - initial_layer,
																						   m - initial_level);

			buffer_image_copy_region_t copy_region;
			copy_region.buffer_offset = buffer_offset_blocks;
			copy_region.image_format = format;
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
	constexpr ste_queue_type queue_type = ste_queue_type::data_transfer_queue;
	const auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
	auto &q = ctx.device().select_queue(queue_selector);

	// Staging buffer
	auto staging_buffer = lib::allocate_unique<staging_buffer_t>(ctx,
																 surface_blocks,
																 buffer_usage::transfer_dst,
																 "host_read_buffer staging buffer");

	// Create copy command
	cmd_copy_image_to_buffer cpy_cmd(image,
									 gl::image_layout::transfer_src_optimal,
									 *staging_buffer,
									 regions);

	// Create a batch
	auto batch = q.allocate_batch();
	auto fence = batch->get_fence_ptr();

	// Enqueue on a transfer queue
	q.enqueue([batch = std::move(batch), staging_buffer = staging_buffer.get(), cpy_cmd = std::move(cpy_cmd), wait_semaphores = std::move(wait_semaphores), signal_semaphores = std::move(signal_semaphores)]() mutable {
		auto &command_buffer = batch->acquire_command_buffer();

		// Record and submit a one-time batch
		{
			auto recorder = command_buffer.record();

			// Copy to staging buffer
			recorder
				<< std::move(cpy_cmd)
				<< cmd_pipeline_barrier(pipeline_barrier(pipeline_stage::transfer,
														 pipeline_stage::host,
														 buffer_memory_barrier(*staging_buffer,
																			   access_flags::transfer_write,
																			   access_flags::host_read)));
		}

		batch->wait_semaphores = std::move(wait_semaphores);
		batch->signal_semaphores = std::move(signal_semaphores);

		ste_device_queue::submit_batch(std::move(batch));
	});

	// Return future that reads from the device buffer
	return ctx.engine().task_scheduler().schedule_now([=, staging_buffer = std::move(staging_buffer), fence = std::move(fence)]() {
		// Create surface
		auto surface = lib::allocate_unique<resource::surface<format, image_type>>(surface_extent,
																				   layers,
																				   mips);

		// Wait for device completion 
		(*fence)->get_wait();

		// mmap
		auto staging_ptr = staging_buffer->get_underlying_memory().template mmap<block_type>(0, surface_blocks);
		// Invalidate caches
		staging_ptr->invalidate_ranges({ vk::vk_mapped_memory_range{ 0, surface_blocks } });

		// Copy from staging
		std::memcpy(surface->data(),
					staging_ptr->get_mapped_ptr(),
					surface_blocks * static_cast<std::size_t>(block_bytes));

		return surface;
	});
}

}

/*
*	@brief	Creates a 1D surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_level		Initial level to start copying from
*	@param	max_level			Last level to copy
*	@param	layer				Image's layer to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_1d(const ste_context &ctx,
						const device_image<1, allocation_policy> &image,
						levels_t initial_level = 0_mip,
						levels_t max_level = all_mips,
						layers_t layer = 0_layer,
						lib::vector<wait_semaphore> &&wait_semaphores = {},
						lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_1d>(ctx,
																	image,
																	layer,
																	initial_level,
																	layer,
																	max_level,
																	std::move(wait_semaphores),
																	std::move(signal_semaphores));
}

/*
*	@brief	Creates a 2D surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_level		Initial level to start copying from
*	@param	max_level			Last level to copy
*	@param	layer				Image's layer to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_2d(const ste_context &ctx,
						const device_image<2, allocation_policy> &image,
						levels_t initial_level = 0_mip,
						levels_t max_level = all_mips,
						layers_t layer = 0_layer,
						lib::vector<wait_semaphore> &&wait_semaphores = {},
						lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_2d>(ctx,
																	image,
																	layer,
																	initial_level,
																	layer,
																	max_level,
																	std::move(wait_semaphores),
																	std::move(signal_semaphores));
}

/*
*	@brief	Creates a 3D surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_level		Initial level to start copying from
*	@param	max_level			Last level to copy
*	@param	layer				Image's layer to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_3d(const ste_context &ctx,
						const device_image<3, allocation_policy> &image,
						levels_t initial_level = 0_mip,
						levels_t max_level = all_mips,
						layers_t layer = 0_layer,
						lib::vector<wait_semaphore> &&wait_semaphores = {},
						lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_3d>(ctx,
																	image,
																	layer,
																	initial_level,
																	layer,
																	max_level,
																	std::move(wait_semaphores),
																	std::move(signal_semaphores));
}

/*
*	@brief	Creates a 1D array surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_layer		Initial layer to start copying from
*	@param	initial_level		Initial level to start copying from
*	@param	max_layer			Last layer to copy
*	@param	max_level			Last level to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_1d_array(const ste_context &ctx,
							  const device_image<1, allocation_policy> &image,
							  layers_t initial_layer = 0_layer,
							  levels_t initial_level = 0_mip,
							  layers_t max_layer = all_layers,
							  levels_t max_level = all_mips,
							  lib::vector<wait_semaphore> &&wait_semaphores = {},
							  lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_1d_array>(ctx,
																		  image,
																		  initial_layer,
																		  initial_level,
																		  max_layer,
																		  max_level,
																		  std::move(wait_semaphores),
																		  std::move(signal_semaphores));
}

/*
*	@brief	Creates a 2D array surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_layer		Initial layer to start copying from
*	@param	initial_level		Initial level to start copying from
*	@param	max_layer			Last layer to copy
*	@param	max_level			Last level to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_2d_array(const ste_context &ctx,
							  const device_image<2, allocation_policy> &image,
							  layers_t initial_layer = 0_layer,
							  levels_t initial_level = 0_mip,
							  layers_t max_layer = all_layers,
							  levels_t max_level = all_mips,
							  lib::vector<wait_semaphore> &&wait_semaphores = {},
							  lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_2d_array>(ctx,
																		  image,
																		  initial_layer,
																		  initial_level,
																		  max_layer,
																		  max_level,
																		  std::move(wait_semaphores),
																		  std::move(signal_semaphores));
}

/*
*	@brief	Creates a cubemap surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_layer		Initial layer to start copying from
*	@param	initial_level		Initial level to start copying from
*	@param	max_layer			Last layer to copy
*	@param	max_level			Last level to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_cubemap(const ste_context &ctx,
							 const device_image<2, allocation_policy> &image,
							 layers_t initial_layer = 0_layer,
							 levels_t initial_level = 0_mip,
							 layers_t max_layer = all_layers,
							 levels_t max_level = all_mips,
							 lib::vector<wait_semaphore> &&wait_semaphores = {},
							 lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_cubemap>(ctx,
																		 image,
																		 initial_layer,
																		 initial_level,
																		 max_layer,
																		 max_level,
																		 std::move(wait_semaphores),
																		 std::move(signal_semaphores));
}

/*
*	@brief	Creates a cubemap array surface and populates it with blocks copies from a device image.
*			Asynchronous call, returns a future. Image must have transfer_src usage flag and be in transfer_src_optimal layout.
*
*	@param	ctx					Context
*	@param	image				Device image to copy from
*	@param	initial_layer		Initial layer to start copying from
*	@param	initial_level		Initial level to start copying from
*	@param	max_layer			Last layer to copy
*	@param	max_level			Last level to copy
*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution
*	@param	signal_semaphores	Sempahores to signal once the command has completed execution
*/
template <gl::format format, class allocation_policy>
auto host_read_image_cubemap_array(const ste_context &ctx,
								   const device_image<2, allocation_policy> &image,
								   layers_t initial_layer = 0_layer,
								   levels_t initial_level = 0_mip,
								   layers_t max_layer = all_layers,
								   levels_t max_level = all_mips,
								   lib::vector<wait_semaphore> &&wait_semaphores = {},
								   lib::vector<semaphore*> &&signal_semaphores = {}) {
	return _internal::host_read_image<format, image_type::image_cubemap_array>(ctx,
																			   image,
																			   initial_layer,
																			   initial_level,
																			   max_layer,
																			   max_level,
																			   std::move(wait_semaphores),
																			   std::move(signal_semaphores));
}

}
}
