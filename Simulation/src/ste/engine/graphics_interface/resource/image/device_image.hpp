//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_exceptions.hpp>
#include <device_image_layout.hpp>

#include <vk_image.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

#include <vk_format_type_traits.hpp>
#include <vk_cmd_pipeline_barrier.hpp>
#include <vk_cmd_copy_image.hpp>

namespace StE {
namespace GL {

template <VkFormat format>
struct device_image_from_surface {};

template <int dimensions, class allocation_policy = device_resource_allocation_policy_device>
class device_image : public device_resource<vk_image<dimensions>, allocation_policy> {
	using Base = device_resource<vk_image<dimensions>, allocation_policy>;

public:
	device_image_layout image_layout;

public:
	template <typename QueueOwnershipArgs, typename ... Args>
	device_image(const ste_context &ctx,
				 QueueOwnershipArgs&& qoa,
				 const vk_image_initial_layout &layout,
				 Args&&... args)
		: Base(ctx,
			   std::forward<QueueOwnershipArgs>(qoa),
			   layout,
			   std::forward<Args>(args)...),
		image_layout(layout)
	{}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;
};

/**
 *	@brief	Partial specialization for 2-dimensional images.
 */
template <class allocation_policy>
class device_image<2, allocation_policy> : public device_resource<vk_image<2>, allocation_policy> {
	using Base = device_resource<vk_image<2>, allocation_policy>;

public:
	device_image_layout image_layout;

private:
	template <VkFormat format>
	static auto create_image_2d(const ste_context &ctx,
								gli::texture2d &&surface,
								const VkImageUsageFlags &usage) {
		using image_element_type = typename GL::vk_format_traits<format>::element_type;
		static constexpr int image_texel_bytes = GL::vk_format_traits<format>::texel_bytes;
		static constexpr gli::format image_gli_format = GL::vk_format_traits<format>::gli_format;
		static constexpr bool image_is_depth = GL::vk_format_traits<format>::is_depth;

		auto src_format = surface.format();
		auto mip_levels = surface.levels();
		auto layers = surface.layers();
		if (src_format != image_gli_format) {
			throw device_image_format_exception("Input format is different from specified image format");
		}

		auto selector = make_data_queue_selector();

		// Create staging image
		GL::device_image<2, GL::device_resource_allocation_policy_host_visible_coherent>
			staging_image(ctx, selector, vk_image_initial_layout::preinitialized, 
						  format, surface.extent(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
						  1, layers, false, false);
		auto staging_image_bytes = staging_image.get_underlying_memory().get_size();
		auto mmap_u8_ptr = staging_image.get_underlying_memory().template mmap<glm::u8>(0, staging_image_bytes);

		auto subresource_layout = staging_image->get_image_subresource_layout(0);

		// Create target image
		GL::device_image<2, GL::device_resource_allocation_policy_device>
			image(ctx, selector, vk_image_initial_layout::unused,
				  format, surface.extent(), VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage,
				  mip_levels, layers);

		// Upload
		for (std::uint32_t m = 0; m < mip_levels; ++m) {
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
					auto barrier = GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															std::vector<GL::vk_image_memory_barrier>{
						GL::vk_image_memory_barrier(staging_image.get(),
													m ==0 ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
													VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
													VK_ACCESS_HOST_WRITE_BIT,
													VK_ACCESS_TRANSFER_READ_BIT),
							GL::vk_image_memory_barrier(image.get(),
														m == 0 ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
														VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
														0,
														VK_ACCESS_TRANSFER_WRITE_BIT) });
					recorder << GL::vk_cmd_pipeline_barrier(barrier);

					// Copy to image
					recorder << GL::vk_cmd_copy_image(staging_image.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
													  image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
													  { range });
				}

				GL::ste_device_queue::submit_batch(std::move(batch));
			});

			// Wait for completion
			fence.get();
		}

		return image;
	}

public:
	template <typename QueueOwnershipArgs, typename ... Args>
	device_image(std::enable_if_t<std::is_constructible_v<vk_image<2>, const vk_logical_device&, const vk_image_initial_layout &, Args...>, const ste_context &> &ctx,
				 QueueOwnershipArgs&& qoa,
				 const vk_image_initial_layout &layout,
				 Args&&... args)
		: Base(ctx,
			   std::forward<QueueOwnershipArgs>(qoa),
			   layout,
			   std::forward<Args>(args)...),
		image_layout(layout)
	{}
	/**
	*	@brief	Constructs a 2D image from a 2D surface, with or without mipmaps. To use this overload, pass device_image_from_surface
	*			as last parameter, indicating the desired type of the resulting image.
	*/
	template <VkFormat format>
	device_image(const ste_context &ctx,
				 const VkImageUsageFlags &usage,
				 gli::texture2d &&surface,
				 const device_image_from_surface<format>&)
		: Base(create_image_2d<format>(ctx,
									   std::move(surface),
									   usage)),
		image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{}
	~device_image() noexcept {}

	device_image(device_image&&) = default;
	device_image &operator=(device_image&&) = default;
};

}
}
