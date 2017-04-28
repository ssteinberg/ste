//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

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
};

constexpr auto operator|(const image_layout &lhs, const image_layout &rhs) {
	using T = std::underlying_type_t<image_layout>;
	return static_cast<image_layout>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const image_layout &lhs, const image_layout &rhs) {
	using T = std::underlying_type_t<image_layout>;
	return static_cast<image_layout>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const image_layout &lhs, const image_layout &rhs) {
	using T = std::underlying_type_t<image_layout>;
	return static_cast<image_layout>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
