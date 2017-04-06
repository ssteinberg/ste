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
		type == ste_shader_stage_variable_type::sampler_t ||
		type == ste_shader_stage_variable_type::texture_t ||
		type == ste_shader_stage_variable_type::opaque_t;
}

template <typename T>
ste_shader_stage_variable_type ste_shader_stage_variable_type_from_type(std::uint32_t *width = nullptr) {
	using Type = std::remove_cv_t<std::remove_reference_t<T>>;

	if (std::is_convertible_v<Type, sampler>)
		return ste_shader_stage_variable_type::sampler_t;
	if (std::is_constructible_v<Type, image_view_generic>)
		return ste_shader_stage_variable_type::image_t;
	if (std::is_constructible_v<Type, texture>)
		return ste_shader_stage_variable_type::texture_t;
	if (std::is_same_v<Type, bool>)
		return ste_shader_stage_variable_type::bool_t;
	if (is_scalar<Type>::value) {
		if (width)
			*width = sizeof(Type) << 3;

		if (is_floating_point<Type>::value)
			return ste_shader_stage_variable_type::float_t;
		if (is_signed<Type>::value)
			return ste_shader_stage_variable_type::int_t;
		return ste_shader_stage_variable_type::uint_t;
	}
	if (std::is_class_v<Type> && std::is_pod_v<Type>)
		return ste_shader_stage_variable_type::struct_t;

	return ste_shader_stage_variable_type::unknown;
}

}
}
