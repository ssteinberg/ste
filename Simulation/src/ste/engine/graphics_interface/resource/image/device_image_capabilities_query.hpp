//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <device_image.hpp>
#include <format.hpp>
#include <image_usage.hpp>
#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <vk_physical_device_descriptor.hpp>

namespace ste {
namespace gl {

template <int dimensions>
struct device_image_capabilities_query_results {
	/**
	*	@brief	Is set to true if any possible combination of extent, levels and layers is allowed for an image of provided type, tiling and format.
	*/
	bool supported{ false };

	/**
	*	@brief	Maximum image dimensions
	*/
	image_extent_type_t<dimensions> max_extent;
	/**
	*	@brief	Maximum number of mipmap levels.
	*			Must either be equal to 1 (valid only if tiling is linear) or be equal to ⌈log2(max(width, height, depth))⌉ + 1. width, height, and depth are taken from the corresponding members of max_extent.
	*/
	std::uint32_t max_levels;
	/**
	*	@brief	Maximum number of array layers.
	*			Must either be equal to 1 or be greater than or equal to the maxImageArrayLayers member of VkPhysicalDeviceLimits. A value of 1 is valid only if tiling is linear or if type is image_3d.
	*/
	std::uint32_t max_layers;

	/**
	*	@brief	Maximum total resource size, in bytes, allowed.
	*/
	std::size_t max_resource_size_bytes;
};

template <int dimensions>
static auto device_image_capabilities_query(const vk::vk_physical_device_descriptor &physical_device,
											format image_format,
											image_usage usage,
											device_image_flags flags = device_image_flags::none) {
	static_assert(1 <= dimensions && dimensions <= 3);

	// Prepare flags
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	auto vk_image_flags = vk::vk_image_default_flags;
	if ((flags & device_image_flags::support_cube_views) != device_image_flags::none)
		vk_image_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	if ((flags & device_image_flags::sparse) != device_image_flags::none)
		vk_image_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	if ((flags & device_image_flags::linear_tiling) != device_image_flags::none)
		tiling = VK_IMAGE_TILING_LINEAR;

	VkImageType type;
	if constexpr (dimensions == 1) type = VK_IMAGE_TYPE_1D;
	if constexpr (dimensions == 2) type = VK_IMAGE_TYPE_2D;
	if constexpr (dimensions == 3) type = VK_IMAGE_TYPE_3D;

	// Query
	const VkImageFormatProperties caps = physical_device.query_physical_device_image_properties(static_cast<VkFormat>(image_format),
																								type,
																								tiling,
																								static_cast<VkImageUsageFlags>(usage),
																								vk_image_flags);

	// Write output
	device_image_capabilities_query_results<dimensions> result;
	result.max_layers = caps.maxArrayLayers;
	result.max_levels = caps.maxMipLevels;
	result.max_resource_size_bytes = caps.maxResourceSize;
	result.supported = result.max_layers > 0 && result.max_levels > 0 && result.max_resource_size_bytes > 0;

	if constexpr (dimensions > 0) {
		result.max_extent.x = caps.maxExtent.width;
		if (result.max_extent.x == 0)
			result.supported = false;
	}
	if constexpr (dimensions > 1) {
		result.max_extent.y = caps.maxExtent.height;
		if (result.max_extent.y == 0)
			result.supported = false;
	}
	if constexpr (dimensions > 2) {
		result.max_extent.z = caps.maxExtent.depth;
		if (result.max_extent.z == 0)
			result.supported = false;
	}

	return result;
}

}
}
