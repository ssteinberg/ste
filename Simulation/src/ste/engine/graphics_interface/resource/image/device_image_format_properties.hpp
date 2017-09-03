//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <format.hpp>
#include <image_usage.hpp>

#include <vk_physical_device_descriptor.hpp>

namespace ste {
namespace gl {

struct device_image_format_query_results {
	bool linear_tiling{ true };
	bool optimal_tiling{ true };
	bool buffer{ true };
};

static auto device_image_format_query(const vk::vk_physical_device_descriptor &physical_device,
									  format image_format,
									  image_usage usage) {
	// Query device format properties
	const auto format_properties = physical_device.query_physical_device_format_properties(static_cast<VkFormat>(image_format));

	// Check features
	device_image_format_query_results result;
	if ((usage & image_usage::transfer_src) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR);
	}
	if ((usage & image_usage::transfer_dst) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR);
	}
	if ((usage & image_usage::sampled) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}
	if ((usage & image_usage::storage) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
	}
	if ((usage & image_usage::color_attachment) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
	}
	if ((usage & image_usage::depth_stencil_attachment) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	if ((usage & image_usage::transient_attachment) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
	}
	if ((usage & image_usage::input_attachment) != static_cast<image_usage>(0)) {
		result.linear_tiling &= !!(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.optimal_tiling &= !!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
		result.buffer &= !!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
	}

	return result;
}

}
}
