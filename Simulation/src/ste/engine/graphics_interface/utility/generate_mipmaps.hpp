//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <device_image.hpp>
#include <image_layout.hpp>

#include <pipeline_barrier.hpp>
#include <image_memory_barrier.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_blit_image.hpp>

#include <format_rtti.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Generate mipmaps for target image.
 *			Assumes image belongs to a primary queue, i.e. ste_queue_type::primary_queue. It is the callers
 *			responsibility to ensure proper queue ownership.
 *
 *	@param	image			Target image
 *	@param	initial_layout	Image initial layout
 *	@param	final_layout		Desired image layout. After job completion image will be in that layout.
 *	@param	start_level		First level to generate mipmaps from
 */
template <int dimensions, class allocation_policy>
auto generate_mipmaps(const device_image<dimensions, allocation_policy> &image,
					  image_layout initial_layout,
					  image_layout final_layout,
					  std::uint32_t start_level = 1) {
	auto future = image.parent_context().engine().task_scheduler().schedule_now([=, &image]() {
		const ste_context &ctx = image.parent_context();

		auto image_format = image.get_format();
		auto mip_levels = image.get_mips();
		auto size = image.get_extent();

		assert(start_level > 0);
		if (mip_levels <= start_level) {
			// Nothing to do
			return;
		}

		// Select queue
		auto queue_type = ste_queue_type::primary_queue;
		auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
		auto &q = ctx.device().select_queue(queue_selector);

		// Enqueue mipmap copy on a queue
		auto enqueue_future = q.enqueue([&]() {
			auto m = std::max<std::uint32_t>(start_level, 1);

			auto batch = ste_device_queue::thread_allocate_batch();
			auto& command_buffer = batch->acquire_command_buffer();

			VkImageAspectFlags aspect = static_cast<VkImageAspectFlags>(format_aspect(image_format));

			// Record and submit a one-time batch
			{
				auto recorder = command_buffer.record();

				for (; m < mip_levels; ++m) {
					// Move to transfer layouts
					auto barrier = pipeline_barrier(m == start_level ? pipeline_stage::bottom_of_pipe : pipeline_stage::transfer,
													pipeline_stage::transfer,
													image_memory_barrier(image,
																		 initial_layout,
																		 image_layout::transfer_dst_optimal,
																		 m, 1, 0, 1),
													image_memory_barrier(image,
																		 m == start_level ? initial_layout : image_layout::transfer_dst_optimal,
																		 image_layout::transfer_src_optimal,
																		 m - 1, 1, 0, 1));
					recorder << cmd_pipeline_barrier(barrier);

					// Blit
					VkImageBlit range = {};
					range.srcSubresource = { aspect, m - 1, 0, 1 };
					range.dstSubresource = { aspect, m, 0, 1 };
					range.srcOffsets[1] = {
						std::max<std::int32_t>(1, size.x >> (m - 1)),
						std::max<std::int32_t>(1, size.y >> (m - 1)),
						std::max<std::int32_t>(1, size.z >> (m - 1)),
					};
					range.dstOffsets[1] = {
						std::max<std::int32_t>(1, size.x >> m),
						std::max<std::int32_t>(1, size.y >> m),
						std::max<std::int32_t>(1, size.z >> m),
					};
					recorder << cmd_blit_image(image, image_layout::transfer_src_optimal,
											   image, image_layout::transfer_dst_optimal,
											   sampler_filter::linear, { range });
				}

				// Move image to final layout
				auto image_barriers = lib::vector<image_memory_barrier>
				{
					image_memory_barrier(image,
										 image_layout::transfer_src_optimal,
										 final_layout,
										 start_level - 1, mip_levels - start_level, 0, 1),
					image_memory_barrier(image,
										 image_layout::transfer_dst_optimal,
										 final_layout,
										 start_level + mip_levels - 2, 1, 0, 1),
				};
				if (start_level > 1) {
					// Transfer untouched mipmap head from initial to final layout as well
					image_barriers.push_back(image_memory_barrier(image,
																  initial_layout,
																  final_layout,
																  0, start_level - 1, 0, 1));
				}
				auto barrier = pipeline_barrier(pipeline_stage::transfer,
												pipeline_stage::top_of_pipe,
												{}, {}, image_barriers);
				recorder << cmd_pipeline_barrier(barrier);
			}

			ste_device_queue::submit_batch(std::move(batch));
		});

		enqueue_future.get();
	});

	return make_job(std::move(future));
}

}
}
