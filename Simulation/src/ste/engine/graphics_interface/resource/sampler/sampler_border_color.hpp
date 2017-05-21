//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class sampler_border_color : std::uint32_t {
	transparent_black_float = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	transparent_black_int = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	opaque_black_float = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	opaque_black_int = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	opaque_white_float = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	opaque_white_int = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
};

constexpr auto operator|(const sampler_border_color &lhs, const sampler_border_color &rhs) {
	using T = std::underlying_type_t<sampler_border_color>;
	return static_cast<sampler_border_color>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const sampler_border_color &lhs, const sampler_border_color &rhs) {
	using T = std::underlying_type_t<sampler_border_color>;
	return static_cast<sampler_border_color>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const sampler_border_color &lhs, const sampler_border_color &rhs) {
	using T = std::underlying_type_t<sampler_border_color>;
	return static_cast<sampler_border_color>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
