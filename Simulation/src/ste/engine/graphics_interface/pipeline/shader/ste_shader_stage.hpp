// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <string>

namespace StE {
namespace GL {

enum class ste_shader_stage : std::uint32_t {
	vertex_program = 0,
	tesselation_control_program = 1,
	tesselation_evaluation_program = 2,
	geometry_program = 3,
	fragment_program = 4,
	compute_program = 5,

	none = 0xFFFFFFFF,
};

}
}
