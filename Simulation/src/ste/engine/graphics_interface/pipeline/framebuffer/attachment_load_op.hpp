//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class attachment_load_op : std::uint32_t {
	load = VK_ATTACHMENT_LOAD_OP_LOAD,
	clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
	undefined = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

constexpr auto operator|(const attachment_load_op &lhs, const attachment_load_op &rhs) {
	using T = std::underlying_type_t<attachment_load_op>;
	return static_cast<attachment_load_op>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const attachment_load_op &lhs, const attachment_load_op &rhs) {
	using T = std::underlying_type_t<attachment_load_op>;
	return static_cast<attachment_load_op>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const attachment_load_op &lhs, const attachment_load_op &rhs) {
	using T = std::underlying_type_t<attachment_load_op>;
	return static_cast<attachment_load_op>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
