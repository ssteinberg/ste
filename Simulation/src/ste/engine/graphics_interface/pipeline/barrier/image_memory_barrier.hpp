//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <device_image.hpp>
#include <ste_queue_family.hpp>
#include <device_image_layout_transformable.hpp>
#include <device_resource_queue_transferable.hpp>

namespace StE {
namespace GL {

class image_memory_barrier {
	friend class cmd_pipeline_barrier;

private:
	VkAccessFlags src;
	VkAccessFlags dst;
	VkImageLayout old_layout;
	VkImageLayout new_layout;
	ste_queue_family src_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	ste_queue_family dst_queue_family{ VK_QUEUE_FAMILY_IGNORED };
	VkImage image;

	const device_image_layout_transformable *image_layout;
	const device_resource_queue_transferable *queue_ownership;

	VkImageAspectFlags aspect;
	std::uint32_t base_level{ 0 };
	std::uint32_t levels{ VK_REMAINING_MIP_LEVELS };
	std::uint32_t base_layer{ 0 };
	std::uint32_t layers{ VK_REMAINING_ARRAY_LAYERS };

public:
	image_memory_barrier(const device_image_base &image,
						 VkImageLayout old_layout,
						 VkImageLayout new_layout,
						 const VkAccessFlags &src_access,
						 const VkAccessFlags &dst_access,
						 bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		image(image.get_image_handle()), image_layout(&image), queue_ownership(&image),
		aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT)
	{}
	image_memory_barrier(const device_image_base &image,
						 VkImageLayout old_layout,
						 VkImageLayout new_layout,
						 const VkAccessFlags &src_access,
						 const VkAccessFlags &dst_access,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers,
						 bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		image(image.get_image_handle()), image_layout(nullptr), queue_ownership(&image),
		aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	image_memory_barrier(const device_image_base &image,
						 VkImageLayout old_layout,
						 VkImageLayout new_layout,
						 const VkAccessFlags &src_access,
						 const VkAccessFlags &dst_access,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family,
						 bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image.get_image_handle()), image_layout(&image), queue_ownership(&image),
		aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT)
	{}
	image_memory_barrier(const device_image_base &image,
						 VkImageLayout old_layout,
						 VkImageLayout new_layout,
						 const VkAccessFlags &src_access,
						 const VkAccessFlags &dst_access,
						 const ste_queue_family &src_queue_family,
						 const ste_queue_family &dst_queue_family,
						 std::uint32_t base_mip_level,
						 std::uint32_t mip_levels,
						 std::uint32_t base_array_layer,
						 std::uint32_t array_layers,
						 bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		src_queue_family(src_queue_family), dst_queue_family(dst_queue_family),
		image(image.get_image_handle()), image_layout(nullptr), queue_ownership(&image),
		aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
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
		barrier.srcAccessMask = src;
		barrier.dstAccessMask = dst;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = static_cast<std::uint32_t>(src_queue_family);
		barrier.dstQueueFamilyIndex = static_cast<std::uint32_t>(dst_queue_family);
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = base_level;
		barrier.subresourceRange.levelCount = levels;
		barrier.subresourceRange.baseArrayLayer = base_layer;
		barrier.subresourceRange.layerCount = layers;

		return barrier;
	}
};

}
}
