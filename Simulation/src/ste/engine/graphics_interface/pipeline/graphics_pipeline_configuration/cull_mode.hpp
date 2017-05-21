//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class cull_mode : std::uint32_t {
	none = VK_CULL_MODE_NONE,
	front_bit = VK_CULL_MODE_FRONT_BIT,
	back_bit = VK_CULL_MODE_BACK_BIT,
	front_and_back = VK_CULL_MODE_FRONT_AND_BACK,
};

constexpr auto operator|(const cull_mode &lhs, const cull_mode &rhs) {
	using T = std::underlying_type_t<cull_mode>;
	return static_cast<cull_mode>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const cull_mode &lhs, const cull_mode &rhs) {
	using T = std::underlying_type_t<cull_mode>;
	return static_cast<cull_mode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const cull_mode &lhs, const cull_mode &rhs) {
	using T = std::underlying_type_t<cull_mode>;
	return static_cast<cull_mode>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
