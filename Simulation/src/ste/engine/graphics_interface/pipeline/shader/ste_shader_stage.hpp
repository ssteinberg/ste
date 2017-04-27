// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <string>

namespace ste {
namespace gl {

enum class ste_shader_stage : std::uint32_t {
	vertex_program = 0,
	tesselation_control_program = 1,
	tesselation_evaluation_program = 2,
	geometry_program = 3,
	fragment_program = 4,
	compute_program = 5,

	none = 0xFFFFFFFF,
};

auto inline ste_shader_stage_to_vk_stage(const ste_shader_stage &stage) {
	switch (stage) {
	case ste_shader_stage::vertex_program:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case ste_shader_stage::tesselation_control_program:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ste_shader_stage::tesselation_evaluation_program:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ste_shader_stage::geometry_program:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ste_shader_stage::fragment_program:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ste_shader_stage::compute_program:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		assert(false);
		return VK_SHADER_STAGE_ALL;
	}
}

}
}
