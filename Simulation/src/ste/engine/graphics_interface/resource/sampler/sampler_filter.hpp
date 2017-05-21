//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class sampler_filter : std::uint32_t {
	nearest = VK_FILTER_NEAREST,
	linear = VK_FILTER_LINEAR,
	cubic_img = VK_FILTER_CUBIC_IMG,
};

constexpr auto operator|(const sampler_filter &lhs, const sampler_filter &rhs) {
	using T = std::underlying_type_t<sampler_filter>;
	return static_cast<sampler_filter>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const sampler_filter &lhs, const sampler_filter &rhs) {
	using T = std::underlying_type_t<sampler_filter>;
	return static_cast<sampler_filter>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const sampler_filter &lhs, const sampler_filter &rhs) {
	using T = std::underlying_type_t<sampler_filter>;
	return static_cast<sampler_filter>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
