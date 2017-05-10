//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class primitive_topology : std::uint32_t {
	point_list = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	line_list = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	line_strip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	triangle_list = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	triangle_strip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	triangle_fan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	line_list_with_adjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	line_strip_with_adjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	triangle_list_with_adjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	triangle_strip_with_adjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	patch_list = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

constexpr auto operator|(const primitive_topology &lhs, const primitive_topology &rhs) {
	using T = std::underlying_type_t<primitive_topology>;
	return static_cast<primitive_topology>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const primitive_topology &lhs, const primitive_topology &rhs) {
	using T = std::underlying_type_t<primitive_topology>;
	return static_cast<primitive_topology>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const primitive_topology &lhs, const primitive_topology &rhs) {
	using T = std::underlying_type_t<primitive_topology>;
	return static_cast<primitive_topology>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
