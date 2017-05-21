//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class attachment_store_op : std::uint32_t {
	store = VK_ATTACHMENT_STORE_OP_STORE,
	discard = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

constexpr auto operator|(const attachment_store_op &lhs, const attachment_store_op &rhs) {
	using T = std::underlying_type_t<attachment_store_op>;
	return static_cast<attachment_store_op>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const attachment_store_op &lhs, const attachment_store_op &rhs) {
	using T = std::underlying_type_t<attachment_store_op>;
	return static_cast<attachment_store_op>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const attachment_store_op &lhs, const attachment_store_op &rhs) {
	using T = std::underlying_type_t<attachment_store_op>;
	return static_cast<attachment_store_op>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
