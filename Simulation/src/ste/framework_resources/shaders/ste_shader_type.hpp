// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <string>

namespace StE {
namespace Resource {

enum class ste_shader_type {
	vertex_program,
	tesselation_control_program,
	tesselation_evaluation_program,
	geometry_program,
	fragment_program,
	compute_program,

	none,
};

auto inline shader_type_from_type_string(const std::string &type) {
	if (type == "vert")
		return ste_shader_type::vertex_program;
	if (type == "frag")
		return ste_shader_type::fragment_program;
	if (type == "geometry")
		return ste_shader_type::geometry_program;
	if (type == "tes")
		return ste_shader_type::tesselation_evaluation_program;
	if (type == "tcs")
		return ste_shader_type::tesselation_control_program;
	if (type == "compute")
		return ste_shader_type::compute_program;

	return ste_shader_type::none;
}

}
}
