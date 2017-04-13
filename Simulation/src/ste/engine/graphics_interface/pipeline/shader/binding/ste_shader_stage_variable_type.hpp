// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_type_traits.hpp>

#include <sampler.hpp>
#include <image_view.hpp>
#include <texture.hpp>

namespace StE {
namespace GL {

enum class ste_shader_stage_variable_type : std::uint16_t {
	unknown,
	void_t,
	bool_t,
	int_t,
	uint_t,
	float_t,
	image_t,
	storage_image_t,
	sampler_t,
	texture_t,
	struct_t,
	opaque_t,
};

/**
*	@brief	Checks if type is opaque
*/
bool inline ste_shader_stage_variable_type_is_opaque(const ste_shader_stage_variable_type &type) {
	return type == ste_shader_stage_variable_type::image_t ||
		type == ste_shader_stage_variable_type::storage_image_t ||
		type == ste_shader_stage_variable_type::sampler_t ||
		type == ste_shader_stage_variable_type::texture_t ||
		type == ste_shader_stage_variable_type::opaque_t;
}

}
}
