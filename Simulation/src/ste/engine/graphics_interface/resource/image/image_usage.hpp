//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class image_usage : std::uint32_t {
	transfer_src = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	transfer_dst = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	sampled = VK_IMAGE_USAGE_SAMPLED_BIT,
	storage = VK_IMAGE_USAGE_STORAGE_BIT,
	color_attachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	depth_stencil_attachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	transient_attachment = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	input_attachment = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
};

constexpr auto operator|(const image_usage &lhs, const image_usage &rhs) {
	using T = std::underlying_type_t<image_usage>;
	return static_cast<image_usage>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const image_usage &lhs, const image_usage &rhs) {
	using T = std::underlying_type_t<image_usage>;
	return static_cast<image_usage>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const image_usage &lhs, const image_usage &rhs) {
	using T = std::underlying_type_t<image_usage>;
	return static_cast<image_usage>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
