//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <job.hpp>

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

template <int dimensions, class allocation_policy>
void fill_image_copy_surface_to_staging(const gli::texture &surface,
										const device_image<dimensions, allocation_policy> &staging,
										std::uint32_t m,
										unsigned faces,
										unsigned base_layer,
										unsigned layers,
										const glm::u32vec3 &extent,
										int image_texel_bytes,
										glm::u8* staging_ptr) {
	for (unsigned f = 0; f < faces; ++f) {
		for (unsigned l = base_layer; l < base_layer + layers; ++l) {
			auto subresource_layout = staging->get_image_subresource_layout(0, f + l * faces);

			auto *ptr = staging_ptr + subresource_layout.offset;
			const void *surface_data = surface.data(l, f, m);
			const auto *src = reinterpret_cast<const glm::u8*>(surface_data);
			std::size_t src_size = surface.size(m);

			std::size_t row_stride = image_texel_bytes * extent.x;
			std::size_t depth_stride = row_stride * extent.y;

			if (subresource_layout.rowPitch == row_stride &&
				subresource_layout.depthPitch == depth_stride) {
				std::size_t level_bytes = static_cast<std::size_t>(extent.z * depth_stride);
				std::memcpy(ptr, src, std::min(src_size, level_bytes));
			}
			else {
				for (std::uint32_t z = 0; z < extent.z; ++z) {
					fill_image_copy_depth_slice(extent,
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
}

template <int dimensions, class allocation_policy, typename selector_policy, typename Surface>
auto fill_image_array(const device_image<dimensions, allocation_policy> &image,
					  Surface &&surface,
					  image_layout final_layout,
					  ste_queue_selector<selector_policy> &&final_queue_selector) {
	auto future = image.parent_context().engine().task_scheduler().schedule_now([=, &image, surface = std::move(surface), final_queue_selector = std::move(final_queue_selector)]() {
		const ste_context &ctx = image.parent_context();
		const gli::texture &surface_generic = surface;

		auto image_format = image.get_format();

		auto image_texel_bytes = format_id(image_format).texel_bytes;
		auto surface_texel_bytes = gli::block_size(surface.format());
		glm::u32vec3 surface_extent = surface_generic.extent();
		glm::u32vec3 image_extent = image.get_extent();

		// Calculate layers and levels to copy
		auto faces = static_cast<std::uint32_t>(surface_generic.max_face());
		if (faces == 0) faces = 1;
		if ((faces != 1 && faces != 6) || surface_generic.base_face() != 0) {
			throw device_image_format_exception("Unsupported surface faces layout");
		}
		auto base_layer = static_cast<std::uint32_t>(faces * surface_generic.base_layer());
		auto base_mip = static_cast<std::uint32_t>(surface_generic.base_level());
		auto layers = std::min(faces * static_cast<std::uint32_t>(surface_generic.layers()), 
							   image.get_layers() - base_layer);
		auto mips = std::min(static_cast<std::uint32_t>(surface_generic.levels()), 
							 image.get_mips() - base_mip);

		// Validate formats
		if (surface_texel_bytes != image_texel_bytes ||
			gli::block_extent(surface.format()) != glm::ivec3(1)) {
			throw device_image_format_exception("Surface and image format mismatch");
		}
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
						  image_format, surface.extent(), image_usage::transfer_src,
						  1, base_layer + layers, false, false);
		auto staging_image_bytes = staging_image.get_underlying_memory().get_size();
		auto mmap_u8_ptr = staging_image.get_underlying_memory().template mmap<glm::u8>(0, staging_image_bytes);

		// Upload
		for (std::uint32_t m = base_mip; m < mips; ++m) {
			auto size = surface_generic.extent(m);

			VkImageAspectFlags aspect = static_cast<VkImageAspectFlags>(format_aspect(image_format));
			VkImageCopy range = {
				{ aspect, 0, base_layer, layers },
				{ 0, 0, 0 },
				{ aspect, m, base_layer, layers },
				{ 0, 0, 0 },
				{ static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y), static_cast<std::uint32_t>(size.z) }
			};

			// Write mipmap level to staging
			fill_image_copy_surface_to_staging(surface_generic,
											   staging_image,
											   m,
											   faces,
											   base_layer / faces,
											   layers / faces,
											   size,
											   image_texel_bytes,
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
					auto barrier = pipeline_barrier(pipeline_stage::top_of_pipe,
													pipeline_stage::top_of_pipe,
													image_memory_barrier(staging_image,
																		 m == 0 ? image_layout::preinitialized : image_layout::transfer_src_optimal,
																		 image_layout::transfer_src_optimal,
																		 access_flags::host_write,
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
			auto queue_transfer_future = queue_transfer(ctx,
														image,
														q,
														ctx.device().select_queue(final_queue_selector),
														image_layout::transfer_dst_optimal, pipeline_stage::transfer,
														final_layout, pipeline_stage::all_commands);
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<1, allocation_policy> &image,
				gli::texture1d &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<1, allocation_policy> &image,
				gli::texture1d_array &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<2, allocation_policy> &image,
				gli::texture2d &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<2, allocation_policy> &image,
				gli::texture2d_array &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<3, allocation_policy> &image,
				gli::texture3d &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<2, allocation_policy> &image,
				gli::texture_cube &&surface,
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
template <class allocation_policy, typename selector_policy>
auto fill_image(const device_image<2, allocation_policy> &image,
				gli::texture_cube_array &&surface,
				image_layout final_layout,
				ste_queue_selector<selector_policy> &&final_queue_selector) {
	return _internal::fill_image_array<>(image,
										 std::move(surface),
										 final_layout,
										 std::move(final_queue_selector));
}

}
}
