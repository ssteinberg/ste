//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

enum class image_layout : std::uint32_t {
	undefined = VK_IMAGE_LAYOUT_UNDEFINED,
	general = VK_IMAGE_LAYOUT_GENERAL,
	color_attachment_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	depth_stencil_attachment_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	depth_stencil_read_only_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	shader_read_only_optimal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	transfer_src_optimal = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	transfer_dst_optimal = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	preinitialized = VK_IMAGE_LAYOUT_PREINITIALIZED,
	present_src_khr = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	shared_present_khr = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
};

/**
 *	@brief	Returns the access flags representing the expected nature of access to an image in a specific image layout.
 */
auto inline access_flags_for_image_layout(image_layout layout) {
	switch (layout) {
	case image_layout::color_attachment_optimal:
		return access_flags::color_attachment_write;
	case image_layout::depth_stencil_attachment_optimal:
		return access_flags::depth_stencil_attachment_write;
	case image_layout::depth_stencil_read_only_optimal:
		return access_flags::depth_stencil_attachment_read | access_flags::shader_read;
	case image_layout::present_src_khr:
		return access_flags::memory_read;
	case image_layout::shader_read_only_optimal:
		return access_flags::shader_read | access_flags::input_attachment_read;
	case image_layout::transfer_dst_optimal:
		return access_flags::transfer_write;
	case image_layout::transfer_src_optimal:
		return access_flags::transfer_read;
	case image_layout::general:
		return access_flags::shader_read | access_flags::shader_write;	// General is used for storage images
	case image_layout::shared_present_khr:
		return access_flags::memory_read | access_flags::transfer_write | access_flags::transfer_read;
	case image_layout::undefined:
	case image_layout::preinitialized:
		return access_flags::none;
	}

	assert(false);
	return access_flags::none;
}

}
}
