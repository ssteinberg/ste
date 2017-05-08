//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class stage_flag : std::uint32_t {
	vertex = VK_SHADER_STAGE_VERTEX_BIT,
	tessellation_control = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	tessellation_evaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
	fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
	compute = VK_SHADER_STAGE_COMPUTE_BIT,
	all_graphics = VK_SHADER_STAGE_ALL_GRAPHICS,
	all = VK_SHADER_STAGE_ALL,

	none = 0,
};

constexpr auto operator|(const stage_flag &lhs, const stage_flag &rhs) {
	using T = std::underlying_type_t<stage_flag>;
	return static_cast<stage_flag>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const stage_flag &lhs, const stage_flag &rhs) {
	using T = std::underlying_type_t<stage_flag>;
	return static_cast<stage_flag>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const stage_flag &lhs, const stage_flag &rhs) {
	using T = std::underlying_type_t<stage_flag>;
	return static_cast<stage_flag>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
