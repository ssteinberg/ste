//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_image.hpp>

namespace StE {
namespace GL {

class vk_image_memory_barrier {
private:
	VkAccessFlags src;
	VkAccessFlags dst;
	VkImageLayout old_layout;
	VkImageLayout new_layout;
	std::uint32_t src_queue_index{ VK_QUEUE_FAMILY_IGNORED };
	std::uint32_t dst_queue_index{ VK_QUEUE_FAMILY_IGNORED };
	VkImage image;

	VkImageAspectFlags aspect;
	std::uint32_t base_level{ 0 };
	std::uint32_t levels{ VK_REMAINING_MIP_LEVELS };
	std::uint32_t base_layer{ 0 };
	std::uint32_t layers{ VK_REMAINING_ARRAY_LAYERS };

public:
	template <int d>
	vk_image_memory_barrier(const vk_image<d> &image,
							VkImageLayout old_layout,
							VkImageLayout new_layout,
							const VkAccessFlags &src_access,
							const VkAccessFlags &dst_access,
							bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		image(image), aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT)
	{}
	template <int d>
	vk_image_memory_barrier(const vk_image<d> &image,
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
		image(image), aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	template <int d>
	vk_image_memory_barrier(const vk_image<d> &image,
							VkImageLayout old_layout,
							VkImageLayout new_layout,
							const VkAccessFlags &src_access,
							const VkAccessFlags &dst_access,
							std::uint32_t src_queue_index,
							std::uint32_t dst_queue_index,
							std::uint32_t base_mip_level,
							std::uint32_t mip_levels,
							std::uint32_t base_array_layer,
							std::uint32_t array_layers,
							bool depth = false)
		: src(src_access), dst(dst_access), old_layout(old_layout), new_layout(new_layout),
		src_queue_index(src_queue_index), dst_queue_index(dst_queue_index),
		image(image), aspect(depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
		base_level(base_mip_level), levels(mip_levels), base_layer(base_array_layer), layers(array_layers)
	{}
	~vk_image_memory_barrier() noexcept {}

	vk_image_memory_barrier(vk_image_memory_barrier &&) = default;
	vk_image_memory_barrier &operator=(vk_image_memory_barrier &&) = default;
	vk_image_memory_barrier(const vk_image_memory_barrier &) = default;
	vk_image_memory_barrier &operator=(const vk_image_memory_barrier &) = default;

	auto &get_src() const { return src; };
	auto &get_dst() const { return dst; };

	operator VkImageMemoryBarrier() const {
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = src;
		barrier.dstAccessMask = dst;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = src_queue_index;
		barrier.dstQueueFamilyIndex = dst_queue_index;
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
