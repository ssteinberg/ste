//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_exceptions.hpp>
#include <device_image_base.hpp>

#include <vk_image.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

#include <vk_format_type_traits.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <cmd_copy_image.hpp>
#include <cmd_blit_image.hpp>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy = device_resource_allocation_policy_device>
class device_image : public device_image_base<dimensions>,
	public device_resource<vk_image<dimensions>, allocation_policy> 
{
	using Base = device_resource<vk_image<dimensions>, allocation_policy>;

public:
	template <typename QueueOwnershipArgs, typename ... Args>
	device_image(const ste_context &ctx,
				 QueueOwnershipArgs&& qoa,
				 const vk_image_initial_layout &layout,
				 Args&&... args)
		: device_image_base(ctx,
							std::forward<QueueOwnershipArgs>(qoa),
							layout),
		Base(ctx,
			 layout,
			 std::forward<Args>(args)...)
	{}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;

	VkImage get_image_handle() const override final { return *this; };
};

/**
 *	@brief	Partial specialization for 2-dimensional images.
 */
template <class allocation_policy>
class device_image<2, allocation_policy> : public device_image_base<2>,
	public device_resource<vk_image<2>, allocation_policy> 
{
	using Base = device_resource<vk_image<2>, allocation_policy>;

private:
	template <typename Image>
	static void copy_surface_to_image(const ste_context &ctx,
									  const Image &image,
									  const gli::texture2d &surface,
									  const VkFormat &format,
									  const ste_queue_selector<> &selector,
									  int image_texel_bytes,
									  bool image_is_depth) {
		auto layers = surface.layers();

		// Create staging image
		device_image<2, device_resource_allocation_policy_host_visible_coherent>
			staging_image(ctx, selector, vk_image_initial_layout::preinitialized,
						  format, surface.extent(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
						  1, layers, false, false);
		auto staging_image_bytes = staging_image.get_underlying_memory().get_size();
		auto mmap_u8_ptr = staging_image.get_underlying_memory().mmap<glm::u8>(0, staging_image_bytes);

		auto subresource_layout = staging_image->get_image_subresource_layout(0);

		// Upload
		for (std::uint32_t m = 0; m < surface.levels(); ++m) {
			auto level = surface[m];

			auto size = level.extent();
			auto *ptr = static_cast<glm::u8*>(*mmap_u8_ptr) + subresource_layout.offset;
			const auto *src = reinterpret_cast<const glm::u8*>(level.data());

			VkImageAspectFlags aspect = image_is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			VkImageCopy range = {
				{ aspect, 0, 0, layers },
				{ 0, 0, 0 },
				{ aspect, m, 0, layers },
				{ 0, 0, 0 },
				{ static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y), 1 }
			};

			// Write mipmap level to staging
			if (subresource_layout.rowPitch == image_texel_bytes * size.x) {
				memcpy(ptr, src, static_cast<std::size_t>(size.y * subresource_layout.rowPitch));
			}
			else {
				for (auto y = 0; y < size.y; ++y) {
					memcpy(ptr, src, static_cast<std::size_t>(image_texel_bytes * size.x));
					ptr += subresource_layout.rowPitch;
					src += size.x * image_texel_bytes;
				}
			}

			// Create a batch
			auto batch = ctx.device().select_queue(selector)->allocate_batch();
			auto& command_buffer = batch->acquire_command_buffer();
			auto& fence = batch->get_fence();

			// Enqueue mipmap copy on a transfer queue
			ctx.device().enqueue(selector, [&]() {
				// Record and submit a one-time batch
				{
					auto recorder = command_buffer.record();

					// Move to transfer layouts
					auto barrier = pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
													VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
													std::vector<image_memory_barrier>{
						image_memory_barrier(staging_image,
											 m == 0 ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
											 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
											 VK_ACCESS_HOST_WRITE_BIT,
											 VK_ACCESS_TRANSFER_READ_BIT),
							image_memory_barrier(image,
												 m == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
												 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
												 0,
												 VK_ACCESS_TRANSFER_WRITE_BIT) });
					recorder << cmd_pipeline_barrier(barrier);

					// Copy to image
					recorder << cmd_copy_image(staging_image.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
											   image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
											   { range });
				}

				ste_device_queue::submit_batch(std::move(batch));

				image.image_layout.layout.store(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, std::memory_order_release);
			});

			// Wait for completion
			fence.get();
		}
	}

	template <typename Image>
	static void generate_image_mipmaps(const ste_context &ctx,
									   const Image &image,
									   const ste_queue_selector<> &selector,
									   const glm::ivec2 &size,
									   std::uint32_t mip_levels,
									   std::uint32_t start_level) {
		// Enqueue mipmap copy on a transfer queue
		auto enqueue_future = ctx.device().enqueue(selector, [&]() {
			auto m = start_level;

			auto batch = ste_device_queue::thread_allocate_batch();
			auto& command_buffer = batch->acquire_command_buffer();

			// Record and submit a one-time batch
			{
				auto recorder = command_buffer.record();

				for (; m < mip_levels; ++m) {
					// Move to transfer layouts
					auto barrier = pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
													VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
													{ image_memory_barrier(image,
																			  VK_IMAGE_LAYOUT_UNDEFINED,
																			  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																			  0,
																			  VK_ACCESS_TRANSFER_WRITE_BIT,
																			  m, 1, 0, 1),
													image_memory_barrier(image,
																			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
																			VK_ACCESS_TRANSFER_WRITE_BIT,
																			VK_ACCESS_TRANSFER_READ_BIT,
																			m - 1, 1, 0, 1) });
					recorder << cmd_pipeline_barrier(barrier);

					VkImageBlit range = {};
					range.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, m - 1, 0, 1 };
					range.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, m, 0, 1 };
					range.srcOffsets[1] = {
						std::max(1, size.x >> (m - 1)),
						std::max(1, size.y >> (m - 1)),
						1
					};
					range.dstOffsets[1] = {
						std::max(1, size.x >> m),
						std::max(1, size.y >> m),
						1
					};

					// Copy to image
					recorder << cmd_blit_image(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
											   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
											   VK_FILTER_LINEAR, { range });
				}

				// Move last mipmap to src optimal layout
				auto barrier = pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
												VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
												image_memory_barrier(image,
																	 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																	 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
																	 VK_ACCESS_TRANSFER_WRITE_BIT,
																	 VK_ACCESS_TRANSFER_READ_BIT,
																	 mip_levels - 1, 1, 0, 1));
				recorder << cmd_pipeline_barrier(barrier);
			}

			ste_device_queue::submit_batch(std::move(batch));

			image.image_layout.layout.store(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, std::memory_order_release);
		});

		enqueue_future.get();
	}

public:
	template <VkFormat format>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&surface,
								const VkImageUsageFlags &usage,
								bool generate_mipmaps) {
		using image_element_type = typename vk_format_traits<format>::element_type;
		static constexpr int image_texel_bytes = vk_format_traits<format>::texel_bytes;
		static constexpr gli::format image_gli_format = vk_format_traits<format>::gli_format;
		static constexpr bool image_is_depth = vk_format_traits<format>::is_depth;

		auto src_format = surface.format();
		std::uint32_t mip_levels = generate_mipmaps ? gli::levels(surface.extent()) : surface.levels();
		auto layers = surface.layers();
		auto size = surface.extent();
		if (src_format != image_gli_format) {
			throw device_image_format_exception("Input format is different from specified image format");
		}

		auto selector = make_primary_queue_selector();

		// Staging area
		device_image<2, device_resource_allocation_policy_device>
			image(ctx, selector, vk_image_initial_layout::unused,
				  format, size, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage,
				  mip_levels, layers);

		// Copy from surface
		copy_surface_to_image(ctx, image, surface, format, selector, image_texel_bytes, image_is_depth);

		// Generate remaining mipmaps, as needed
		std::uint32_t m = surface.levels();
		if (m < mip_levels) {
			generate_image_mipmaps(ctx,
								   image,
								   selector,
								   size,
								   mip_levels,
								   m);
		}

		return image;
	}

public:
	template <typename QueueOwnershipArgs, typename ... Args>
	device_image(std::enable_if_t<std::is_constructible_v<vk_image<2>, const vk_logical_device&, const vk_image_initial_layout &, Args...>, const ste_context &> &ctx,
				 QueueOwnershipArgs&& qoa,
				 const vk_image_initial_layout &layout,
				 Args&&... args)
		: device_image_base(ctx,
							std::forward<QueueOwnershipArgs>(qoa),
							layout),
		Base(ctx,
			 layout,
			 std::forward<Args>(args)...)
	{}
	~device_image() noexcept {}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;

	VkImage get_image_handle() const override final { return *this; };
};

}
}
