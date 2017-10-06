//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <block_layout.hpp>

namespace ste {
namespace gl {

static constexpr auto std430_block_layout_base_alignment = 1_B;

template <typename... Ts>
using std430 = block_layout<1, Ts...>;

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
struct is_std430_layout {
	static constexpr bool value = _detail::is_std430_block_layout_helper::val<T>();
};
template <typename T>
static constexpr bool is_std430_layout_v = is_std430_layout<T>::value;

}
}
