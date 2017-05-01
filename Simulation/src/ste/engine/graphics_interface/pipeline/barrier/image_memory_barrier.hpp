//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <device_image_base.hpp>
#include <ste_queue_family.hpp>

#include <image_layout.hpp>
#include <image_aspect.hpp>
#include <format_rtti.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

class image_memory_barrier {
	friend class cmd_pipeline_barrier;

private:
	access_flags src;
	access_flags dst;
	image_layout old_layout;
	image_layout new_layout;
	ste_queue_family src_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	ste_queue_family dst_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	std::reference_wrapper<const device_image_base> image;

	image_aspect aspect;
	std::uint32_t base_level{ 0 };
	std::uint32_t levels{ VK_REMAINING_MIP_LEVELS };
	std::uint32_t base_layer{ 0 };
	std::uint32_t layers{ VK_REMAINING_ARRAY_LAYERS };

public:
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout)
		: src(access_flags_for_image_layout(old_layout)), dst(access_flags_for_image_layout(new_layout)),
		old_layout(old_layout), new_layout(new_layout),
		image(image),
		aspect(format_aspect(image.get_format()))
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const access_flags &src_access,
						 const access_flags &dst_access)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		image(image),
		aspect(format_aspect(image.get_format()))
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers)
		: src(access_flags_for_image_layout(old_layout)), dst(access_flags_for_image_layout(new_layout)), 
		old_layout(old_layout), new_layout(new_layout),
		image(image),
		aspect(format_aspect(image.get_format())),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const access_flags &src_access,
						 const access_flags &dst_access,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		image(image),
		aspect(format_aspect(image.get_format())),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family)
		: src(access_flags_for_image_layout(old_layout)), dst(access_flags_for_image_layout(new_layout)),
		old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image),
		aspect(format_aspect(image.get_format()))
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const access_flags &src_access,
						 const access_flags &dst_access,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image),
		aspect(format_aspect(image.get_format()))
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const access_flags &src_access,
						 const access_flags &dst_access,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image),
		aspect(format_aspect(image.get_format())),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	image_memory_barrier(const device_image_base &image,
						 image_layout old_layout,
						 image_layout new_layout,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers)
		: src(access_flags_for_image_layout(old_layout)), dst(access_flags_for_image_layout(new_layout)), 
		old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image),
		aspect(format_aspect(image.get_format())),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	~image_memory_barrier() noexcept {}

	image_memory_barrier(image_memory_barrier &&) = default;
	image_memory_barrier &operator=(image_memory_barrier &&) = default;
	image_memory_barrier(const image_memory_barrier &) = default;
	image_memory_barrier &operator=(const image_memory_barrier &) = default;

	operator VkImageMemoryBarrier() const {
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = static_cast<VkAccessFlags>(src);
		barrier.dstAccessMask = static_cast<VkAccessFlags>(dst);
		barrier.oldLayout = static_cast<VkImageLayout>(old_layout);
		barrier.newLayout = static_cast<VkImageLayout>(new_layout);
		barrier.srcQueueFamilyIndex = static_cast<std::uint32_t>(src_queue_family);
		barrier.dstQueueFamilyIndex = static_cast<std::uint32_t>(dst_queue_family);
		barrier.image = image.get().get_image_handle();
		barrier.subresourceRange.aspectMask = static_cast<VkImageAspectFlags>(aspect);
		barrier.subresourceRange.baseMipLevel = base_level;
		barrier.subresourceRange.levelCount = levels;
		barrier.subresourceRange.baseArrayLayer = base_layer;
		barrier.subresourceRange.layerCount = layers;

		return barrier;
	}
};

}
}
