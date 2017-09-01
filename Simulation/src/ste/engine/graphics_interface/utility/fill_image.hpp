//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <job.hpp>

#include <surface.hpp>

#include <device_image.hpp>
#include <device_image_queue_transfer.hpp>
#include <device_image_exceptions.hpp>

#include <pipeline_barrier.hpp>
#include <image_memory_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_copy_image.hpp>

#include <format_rtti.hpp>

#include <cstring>

namespace ste {
namespace gl {

namespace _internal {

void inline fill_image_copy_depth_slice(const glm::u32vec3 &extent,
										std::size_t src_size,
										std::size_t row_pitch,
										std::size_t row_stride,
										glm::u8* ptr,
										const glm::u8* src) {
	for (std::uint32_t y = 0; y < extent.y; ++y) {
		std::memcpy(ptr, src, std::min(src_size, row_stride));
		ptr += row_pitch;
		src += row_stride;
		src_size = src_size > row_stride ? src_size - row_stride : 0;
	}
}

template <int dimensions, class allocation_policy, typename Surface>
void fill_image_copy_surface_to_staging(const Surface &surface,
										const device_image<dimensions, allocation_policy> &staging,
										std::uint32_t level,
										std::uint32_t layers,
										const glm::u32vec3 &extent_blocks,
										int image_block_bytes,
										glm::u8* staging_ptr) {
	for (std::uint32_t l = 0; l < layers; ++l) {
		auto subresource_layout = staging->get_image_subresource_layout(0, l);

		auto *ptr = staging_ptr + subresource_layout.offset;
		const void *surface_data = surface.data_at(l, level);
		const auto *src = reinterpret_cast<const glm::u8*>(surface_data);
		std::size_t src_size = surface.bytes(level);

		const std::size_t row_stride = image_block_bytes * extent_blocks.x;
		const std::size_t depth_stride = row_stride * extent_blocks.y;

		if (subresource_layout.rowPitch == row_stride &&
			subresource_layout.depthPitch == depth_stride) {
			const std::size_t level_bytes = static_cast<std::size_t>(extent_blocks.z * depth_stride);
			std::memcpy(ptr, src, std::min(src_size, level_bytes));
		}
		else {
			for (std::uint32_t z = 0; z < extent_blocks.z; ++z) {
				fill_image_copy_depth_slice(extent_blocks,
											src_size,
											static_cast<std::size_t>(subresource_layout.rowPitch),
											row_stride,
											ptr, src);
				ptr += subresource_layout.depthPitch;
				src += depth_stride;
				src_size = src_size > depth_stride ? src_size - depth_stride : 0;
			}
		}
	}
}

template <int dimensions, class allocation_policy, typename selector_policy, typename Surface>
auto fill_image_array(const device_image<dimensions, allocation_policy> &image,
					  Surface &&surface,
					  image_layout final_layout,
					  ste_queue_selector<selector_policy> &&final_queue_selector) {
	static constexpr gl::format format = Surface::surface_format;
	static constexpr gl::image_type image_type = Surface::surface_image_type;
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

		const auto image_block_bytes = format_id(image_format).block_bytes;
		const auto surface_block_bytes = surface.block_bytes();
		const extent_type image_extent = image.get_extent();
		const extent_type surface_extent = surface.extent();

		// Calculate layers and levels to copy
		auto layers = std::min(static_cast<std::uint32_t>(surface.layers()),
							   image.get_layers());
		auto mips = std::min(static_cast<std::uint32_t>(surface.levels()),
							 image.get_mips());

		// Validate size
		if (surface_extent != image_extent) {
			throw device_image_format_exception("Surface and image extent mismatch");
		}

		// Select queue
		auto queue_type = ste_queue_type::data_transfer_queue;
		auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
		auto &q = ctx.device().select_queue(queue_selector);

		// Create staging image
		device_image<dimensions, device_resource_allocation_policy_host_visible_coherent>
			staging_image(ctx, image_initial_layout::preinitialized,
						  image_format, surface_extent, image_usage::transfer_src,
						  1, layers, false, false);
		auto staging_image_bytes = staging_image.get_underlying_memory().get_size();
		auto mmap_u8_ptr = staging_image.get_underlying_memory().template mmap<glm::u8>(0, staging_image_bytes);

		// Upload
		for (std::uint32_t m = 0; m < mips; ++m) {
			auto extent = surface.extent(m);
			auto extent_blocks = surface.extent_in_blocks(m);
			glm::u32vec3 extent_blocks3 = glm::u32vec3(1);
			extent_blocks3.x = extent_blocks.x;
			if constexpr (dimensions > 1) extent_blocks3.y = extent_blocks[1];
			if constexpr (dimensions > 2) extent_blocks3.z = extent_blocks[2];

			VkImageAspectFlags aspect = static_cast<VkImageAspectFlags>(format_aspect(image_format));
			VkImageCopy range = {
				{ aspect, 0, 0, layers },
				{ 0, 0, 0 },
				{ aspect, m, 0, layers },
				{ 0, 0, 0 },
				{ static_cast<std::uint32_t>(extent.x), 0, 0 }
			};
			if constexpr (dimensions > 1) range.extent.height = static_cast<std::uint32_t>(extent[1]);
			if constexpr (dimensions > 2) range.extent.depth = static_cast<std::uint32_t>(extent[2]);

			// Write mipmap level to staging
			fill_image_copy_surface_to_staging(surface,
											   staging_image,
											   m,
											   layers,
											   extent_blocks3,
											   image_block_bytes,
											   static_cast<glm::u8*>(*mmap_u8_ptr));

			// Create a batch
			auto batch = q.allocate_batch();
			auto& command_buffer = batch->acquire_command_buffer();
			auto fence = batch->get_fence_ptr();

			// Enqueue mipmap copy on a transfer queue
			auto f = q.enqueue([&]() {
				// Record and submit a one-time batch
				{
					auto recorder = command_buffer.record();

					// Move to transfer layouts
					auto barrier = pipeline_barrier(pipeline_stage::top_of_pipe | pipeline_stage::host,
													pipeline_stage::transfer,
													image_memory_barrier(staging_image,
																		 m == 0 ? image_layout::preinitialized : image_layout::transfer_src_optimal,
																		 image_layout::transfer_src_optimal,
																		 m == 0 ? access_flags::none : access_flags::host_write,
																		 access_flags::transfer_read),
													image_memory_barrier(image,
																		 m == 0 ? image_layout::undefined : image_layout::transfer_dst_optimal,
																		 image_layout::transfer_dst_optimal,
																		 access_flags::none,
																		 access_flags::transfer_write));
					recorder << cmd_pipeline_barrier(barrier);

					// Copy to image
					recorder << cmd_copy_image(staging_image, image_layout::transfer_src_optimal,
											   image, image_layout::transfer_dst_optimal,
											   { range });
				}

				ste_device_queue::submit_batch(std::move(batch));
			});
			f.get();

			// Transfer ownership
			pipeline_stage pipeline_stages_for_final_layout = all_possible_pipeline_stages_for_access_flags(access_flags_for_image_layout(final_layout));
			auto queue_transfer_future = queue_transfer(ctx,
														image,
														q,
														ctx.device().select_queue(final_queue_selector),
														image_layout::transfer_dst_optimal, pipeline_stage::transfer,
														final_layout, pipeline_stages_for_final_layout);
			// Wait for completion
			(*fence)->get_wait();
			queue_transfer_future.get();
		}
	});

	return make_job(std::move(future));
}

}

/**
*	@brief	Copies image from surface into target 1d image.
*
*	@param	image			Target image
*	@param	surface			1d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<1, allocation_policy> &image,
				resource::surface_1d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface into target 1d array image.
*
*	@param	image			Target image
*	@param	surface			1d array surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<1, allocation_policy> &image,
				resource::surface_1d_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface into target 2d image.
*
*	@param	image			Target image
*	@param	surface			2d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_2d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface into target 2d array image.
*
*	@param	image			Target image
*	@param	surface			2d array surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_2d_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface into target 3d image.
*
*	@param	image			Target image
*	@param	surface			3d surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<3, allocation_policy> &image,
				resource::surface_3d<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface cubemap into target 2d array image.
*
*	@param	image			Target image
*	@param	surface			Cubemap surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_cubemap<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

/**
*	@brief	Copies image from surface cubemap into target 2d array image.
*
*	@param	image			Target image
*	@param	surface			Array cubemap surface to fill from
*	@param	final_layout		Desired image layout. After job completion image will be in that layout.
*	@param	final_queue_selector		After job completion image will be transfered to this queue
*/
template <class allocation_policy, typename selector_policy, gl::format format>
auto fill_image(const device_image<2, allocation_policy> &image,
				resource::surface_cubemap_array<format> &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

}
}
