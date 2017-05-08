//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class colorspace : std::uint32_t {
	srgb_nonlinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	display_p3_nonlinear = VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
	extended_srgb_linear = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
	dci_p3_linear = VK_COLOR_SPACE_DCI_P3_LINEAR_EXT,
	dci_p3_nonlinear = VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
	bt709_linear = VK_COLOR_SPACE_BT709_LINEAR_EXT,
	bt709_nonlinear = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
	bt2020_linear = VK_COLOR_SPACE_BT2020_LINEAR_EXT,
	hdr10_st2084 = VK_COLOR_SPACE_HDR10_ST2084_EXT,
	dolbyvision = VK_COLOR_SPACE_DOLBYVISION_EXT,
	hdr10_hlg = VK_COLOR_SPACE_HDR10_HLG_EXT,
	adobergb_linear = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
	adobergb_nonlinear = VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
	pass_through = VK_COLOR_SPACE_PASS_THROUGH_EXT,
};

constexpr auto operator|(const colorspace &lhs, const colorspace &rhs) {
	using T = std::underlying_type_t<colorspace>;
	return static_cast<colorspace>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const colorspace &lhs, const colorspace &rhs) {
	using T = std::underlying_type_t<colorspace>;
	return static_cast<colorspace>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const colorspace &lhs, const colorspace &rhs) {
	using T = std::underlying_type_t<colorspace>;
	return static_cast<colorspace>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
