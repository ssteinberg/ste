// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <stage_flag.hpp>

namespace ste {
namespace gl {

enum class ste_shader_program_stage : std::uint32_t {
	vertex_program = 0,
	tessellation_control_program = 1,
	tessellation_evaluation_program = 2,
	geometry_program = 3,
	fragment_program = 4,
	compute_program = 5,

	none = 0xFFFFFFFF,
};

auto inline ste_shader_program_stage_to_stage_flag(const ste_shader_program_stage &stage) {
	switch (stage) {
	case ste_shader_program_stage::vertex_program:
		return stage_flag::vertex;
	case ste_shader_program_stage::tessellation_control_program:
		return stage_flag::tessellation_control;
	case ste_shader_program_stage::tessellation_evaluation_program:
		return stage_flag::tessellation_evaluation;
	case ste_shader_program_stage::geometry_program:
		return stage_flag::geometry;
	case ste_shader_program_stage::fragment_program:
		return stage_flag::fragment;
	case ste_shader_program_stage::compute_program:
		return stage_flag::compute;
	default:
		assert(false);
		return stage_flag::none;
	}
}

}
}
