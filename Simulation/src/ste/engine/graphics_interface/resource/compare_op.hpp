//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class compare_op : std::uint32_t {
	never = VK_COMPARE_OP_NEVER,
	less = VK_COMPARE_OP_LESS,
	equal = VK_COMPARE_OP_EQUAL,
	less_or_equal = VK_COMPARE_OP_LESS_OR_EQUAL,
	greater = VK_COMPARE_OP_GREATER,
	not_equal = VK_COMPARE_OP_NOT_EQUAL,
	greater_or_equal = VK_COMPARE_OP_GREATER_OR_EQUAL,
	always = VK_COMPARE_OP_ALWAYS,
};

constexpr auto operator|(const compare_op &lhs, const compare_op &rhs) {
	using T = std::underlying_type_t<compare_op>;
	return static_cast<compare_op>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const compare_op &lhs, const compare_op &rhs) {
	using T = std::underlying_type_t<compare_op>;
	return static_cast<compare_op>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const compare_op &lhs, const compare_op &rhs) {
	using T = std::underlying_type_t<compare_op>;
	return static_cast<compare_op>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
