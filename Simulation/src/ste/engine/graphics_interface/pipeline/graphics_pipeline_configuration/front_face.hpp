//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class front_face : std::uint32_t {
	ccw = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	cw = VK_FRONT_FACE_CLOCKWISE,
};

constexpr auto operator|(const front_face &lhs, const front_face &rhs) {
	using T = std::underlying_type_t<front_face>;
	return static_cast<front_face>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const front_face &lhs, const front_face &rhs) {
	using T = std::underlying_type_t<front_face>;
	return static_cast<front_face>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const front_face &lhs, const front_face &rhs) {
	using T = std::underlying_type_t<front_face>;
	return static_cast<front_face>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
