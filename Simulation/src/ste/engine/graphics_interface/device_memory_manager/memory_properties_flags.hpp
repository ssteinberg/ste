//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class memory_properties_flags : std::uint32_t {
	none = 0,
	device_local = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	host_visible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	host_coherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	host_cached = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
	lazily_allocated = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
};

constexpr auto operator|(const memory_properties_flags &lhs, const memory_properties_flags &rhs) {
	using T = std::underlying_type_t<memory_properties_flags>;
	return static_cast<memory_properties_flags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const memory_properties_flags &lhs, const memory_properties_flags &rhs) {
	using T = std::underlying_type_t<memory_properties_flags>;
	return static_cast<memory_properties_flags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const memory_properties_flags &lhs, const memory_properties_flags &rhs) {
	using T = std::underlying_type_t<memory_properties_flags>;
	return static_cast<memory_properties_flags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
