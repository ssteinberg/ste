//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class sampler_mipmap_mode : std::uint32_t {
	nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

constexpr auto operator|(const sampler_mipmap_mode &lhs, const sampler_mipmap_mode &rhs) {
	using T = std::underlying_type_t<sampler_mipmap_mode>;
	return static_cast<sampler_mipmap_mode>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const sampler_mipmap_mode &lhs, const sampler_mipmap_mode &rhs) {
	using T = std::underlying_type_t<sampler_mipmap_mode>;
	return static_cast<sampler_mipmap_mode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const sampler_mipmap_mode &lhs, const sampler_mipmap_mode &rhs) {
	using T = std::underlying_type_t<sampler_mipmap_mode>;
	return static_cast<sampler_mipmap_mode>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
