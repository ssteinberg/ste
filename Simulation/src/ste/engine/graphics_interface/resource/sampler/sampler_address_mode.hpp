//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class sampler_address_mode : std::uint32_t {
	repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	mirrored_repeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	clamp_to_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	mirror_clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

constexpr auto operator|(const sampler_address_mode &lhs, const sampler_address_mode &rhs) {
	using T = std::underlying_type_t<sampler_address_mode>;
	return static_cast<sampler_address_mode>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const sampler_address_mode &lhs, const sampler_address_mode &rhs) {
	using T = std::underlying_type_t<sampler_address_mode>;
	return static_cast<sampler_address_mode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const sampler_address_mode &lhs, const sampler_address_mode &rhs) {
	using T = std::underlying_type_t<sampler_address_mode>;
	return static_cast<sampler_address_mode>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
