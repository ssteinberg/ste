//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <block_layout.hpp>

namespace StE {
namespace GL {

static constexpr int std430_block_layout_base_alignment = 1;

template <typename... Ts>
struct std430_layout : block_layout<1, Ts...> {
};

namespace _detail {

struct is_std430_block_layout_helper {
	template <typename T>
	static constexpr bool val(std::enable_if_t<!is_block_layout_v<T>>* = nullptr) { return false; }
	template <typename T>
	static constexpr bool val(std::enable_if_t<is_block_layout_v<T>>* = nullptr) {
		return T::block_base_alignment == std430_block_layout_base_alignment;
	}
};

}

template <typename T>
struct is_std430_block_layout {
	static constexpr bool value = _detail::is_std430_block_layout_helper::val<T>();
};
template <typename T>
static constexpr bool is_std430_block_layout_v = is_std430_block_layout<T>::value;

}
}
