//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class blend_op : std::uint32_t {
	add = VK_BLEND_OP_ADD,
	subtract = VK_BLEND_OP_SUBTRACT,
	reverse_subtract = VK_BLEND_OP_REVERSE_SUBTRACT,
	min = VK_BLEND_OP_MIN,
	max = VK_BLEND_OP_MAX,
};

constexpr auto operator|(const blend_op &lhs, const blend_op &rhs) {
	using T = std::underlying_type_t<blend_op>;
	return static_cast<blend_op>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const blend_op &lhs, const blend_op &rhs) {
	using T = std::underlying_type_t<blend_op>;
	return static_cast<blend_op>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const blend_op &lhs, const blend_op &rhs) {
	using T = std::underlying_type_t<blend_op>;
	return static_cast<blend_op>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
