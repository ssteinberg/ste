//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class color_component : std::uint32_t {
	r = VK_COLOR_COMPONENT_R_BIT,
	g = VK_COLOR_COMPONENT_G_BIT,
	b = VK_COLOR_COMPONENT_B_BIT,
	a = VK_COLOR_COMPONENT_A_BIT,
};

constexpr auto operator|(const color_component &lhs, const color_component &rhs) {
	using T = std::underlying_type_t<color_component>;
	return static_cast<color_component>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const color_component &lhs, const color_component &rhs) {
	using T = std::underlying_type_t<color_component>;
	return static_cast<color_component>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const color_component &lhs, const color_component &rhs) {
	using T = std::underlying_type_t<color_component>;
	return static_cast<color_component>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
