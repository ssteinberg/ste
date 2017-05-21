//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class image_aspect : std::uint32_t {
	color = VK_IMAGE_ASPECT_COLOR_BIT,
	depth = VK_IMAGE_ASPECT_DEPTH_BIT,
	stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
	metadata = VK_IMAGE_ASPECT_METADATA_BIT,
};

constexpr auto operator|(const image_aspect &lhs, const image_aspect &rhs) {
	using T = std::underlying_type_t<image_aspect>;
	return static_cast<image_aspect>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const image_aspect &lhs, const image_aspect &rhs) {
	using T = std::underlying_type_t<image_aspect>;
	return static_cast<image_aspect>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const image_aspect &lhs, const image_aspect &rhs) {
	using T = std::underlying_type_t<image_aspect>;
	return static_cast<image_aspect>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
