//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class blend_factor : std::uint32_t {
	zero = VK_BLEND_FACTOR_ZERO,
	one = VK_BLEND_FACTOR_ONE,
	src_color = VK_BLEND_FACTOR_SRC_COLOR,
	one_minus_src_color = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	dst_color = VK_BLEND_FACTOR_DST_COLOR,
	one_minus_dst_color = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	src_alpha = VK_BLEND_FACTOR_SRC_ALPHA,
	one_minus_src_alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	dst_alpha = VK_BLEND_FACTOR_DST_ALPHA,
	one_minus_dst_alpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	constant_color = VK_BLEND_FACTOR_CONSTANT_COLOR,
	one_minus_constant_color = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	constant_alpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
	one_minus_constant_alpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	src_alpha_saturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	src1_color = VK_BLEND_FACTOR_SRC1_COLOR,
	one_minus_src1_color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	src1_alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
	one_minus_src1_alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

constexpr auto operator|(const blend_factor &lhs, const blend_factor &rhs) {
	using T = std::underlying_type_t<blend_factor>;
	return static_cast<blend_factor>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const blend_factor &lhs, const blend_factor &rhs) {
	using T = std::underlying_type_t<blend_factor>;
	return static_cast<blend_factor>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const blend_factor &lhs, const blend_factor &rhs) {
	using T = std::underlying_type_t<blend_factor>;
	return static_cast<blend_factor>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
