// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_image.hpp>

#include <vk_format_type_traits.hpp>

#include <surface_io.hpp>
#include <surface_mipmap_generator.hpp>
#include <surface_convert.hpp>

#include <vk_cmd_pipeline_barrier.hpp>
#include <vk_cmd_copy_image.hpp>

namespace StE {
namespace Resource {

class surface_factory {
public:
	template <VkFormat format>
	static auto create_image_2d(const ste_context &ctx,
								const gli::texture2d &input,
								const VkImageUsageFlags &usage,
								const VkImageLayout &layout) {
		using image_element_type = typename GL::vk_format_traits<format>::element_type;
		static constexpr int image_element_count = GL::vk_format_traits<format>::elements;
		static constexpr int image_texel_bytes = GL::vk_format_traits<format>::texel_bytes;
		static constexpr gli::format image_gli_format = GL::vk_format_traits<format>::gli_format;

		// Convert surface to target format
		bool need_conversion = image_gli_format != input.format();
		gli::texture2d converted;
		if (need_conversion)
			converted = surface_convert()(input, image_gli_format);
		const gli::texture2d &surface = need_conversion ? converted : input;

		auto src_format = surface.format();
		auto mip_levels = surface.levels();
		if (gli::is_packed(src_format)) {
			ste_log_error() << "Input surface has a packed format." << std::endl;
			throw surface_unsupported_format_error("Input has a packed format");
		}

		// Create staging image
		GL::device_image<2, GL::device_resource_allocation_policy_host_visible_coherent>
			staging_image(ctx, format, surface.extent(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
						  1, 1, false, true, false);
		auto staging_image_bytes = staging_image.get_underlying_memory().get_size();
		auto mmap_u8_ptr = staging_image.get_underlying_memory().template mmap<glm::u8>(0, staging_image_bytes);

		auto subresource_layout = staging_image->get_image_subresource_layout(0);

		// Create target image
		GL::device_image<2, GL::device_resource_allocation_policy_device>
			image(ctx, format, surface.extent(), VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage,
				  mip_levels, 1, true);

		// Select queue
		auto queue_type = GL::ste_queue_type::data_transfer_queue;
		auto queue_selector = GL::ste_queue_selector<GL::ste_queue_selector_policy_flexible>(queue_type);

		// Upload
		for (std::uint32_t m = 0; m < mip_levels; ++m) {
			auto level = surface[m];

			auto size = level.extent();
			auto *ptr = static_cast<glm::u8*>(*mmap_u8_ptr) + subresource_layout.offset;
			const auto *src = reinterpret_cast<const glm::u8*>(level.data());

			VkImageCopy range = {
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
				{ 0, 0, 0 },
				{ VK_IMAGE_ASPECT_COLOR_BIT, m, 0, 1 },
				{ 0, 0, 0 },
				{ static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y), 1 }
			};

			// Write mipmap level to staging
			for (auto y = 0; y < size.y; ++y) {
				memcpy(ptr, src, static_cast<std::size_t>(image_texel_bytes * size.x));
				ptr += subresource_layout.rowPitch;
				src += size.x * image_texel_bytes;
			}

			// Create a batch
			auto batch = ctx.device().select_queue(queue_selector)->allocate_batch();
			auto& command_buffer = batch->acquire_command_buffer();
			auto& fence = batch->get_fence();

			// Enqueue mipmap copy on a transfer queue
			ctx.device().enqueue(queue_selector, [&]() {
				// Record and submit a one-time batch
				{
					auto recorder = command_buffer.record();

					if (m == 0) {
						// Move to transfer layouts
						auto barrier = GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															   std::vector<GL::vk_image_memory_barrier>{
							GL::vk_image_memory_barrier(staging_image.get(),
														VK_IMAGE_LAYOUT_PREINITIALIZED,
														VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
														VK_ACCESS_HOST_WRITE_BIT,
														VK_ACCESS_TRANSFER_READ_BIT),
								GL::vk_image_memory_barrier(image.get(),
															VK_IMAGE_LAYOUT_UNDEFINED,
															VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
															0,
															VK_ACCESS_TRANSFER_WRITE_BIT) });
						recorder << GL::vk_cmd_pipeline_barrier(barrier);
					}

					// Copy to image
					recorder << GL::vk_cmd_copy_image(staging_image.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
													  image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
													  { range });

					if (m == mip_levels - 1) {
						// Move to final layout
						auto barrier = GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
															   GL::vk_image_memory_barrier(image.get(),
																						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
																						   layout,
																						   VK_ACCESS_TRANSFER_WRITE_BIT,
																						   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT));
						recorder << GL::vk_cmd_pipeline_barrier(barrier);
					}
				}

				GL::ste_device_queue::submit_batch(std::move(batch));
			});

			// Wait for completion
			fence.get();
		}

		return image;
	}

	template <VkFormat format>
	static auto create_image_2d(const ste_context &ctx,
								const boost::filesystem::path &path,
								const VkImageUsageFlags &usage,
								const VkImageLayout &layout,
								bool generate_mipmaps = true) {
		auto surface = surface_io::load_surface_2d(path, false);
		if (generate_mipmaps)
			surface = surface_mipmap_generator()(std::move(surface));
		return create_image_2d<format>(ctx, surface, usage, layout);
	}

	template <VkFormat format>
	static auto create_image_2d_srgb(const ste_context &ctx,
									 const boost::filesystem::path &path,
									 const VkImageUsageFlags &usage,
									 const VkImageLayout &layout,
									 bool generate_mipmaps = true) {
		auto surface = surface_io::load_surface_2d(path, true);
		if (generate_mipmaps)
			surface = surface_mipmap_generator()(std::move(surface));
		return create_image_2d<format>(ctx, surface, usage, layout);
	}
};

}
}
