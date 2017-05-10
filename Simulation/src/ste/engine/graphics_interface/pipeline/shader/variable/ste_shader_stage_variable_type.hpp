// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_type_traits.hpp>

#include <block_layout_common.hpp>
#include <sampler.hpp>
#include <image_view.hpp>
#include <texture.hpp>

namespace ste {
namespace gl {

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
constexpr bool ste_shader_stage_variable_type_is_opaque(const ste_shader_stage_variable_type &type) {
	return type == ste_shader_stage_variable_type::image_t ||
		type == ste_shader_stage_variable_type::storage_image_t ||
		type == ste_shader_stage_variable_type::sampler_t ||
		type == ste_shader_stage_variable_type::texture_t ||
		type == ste_shader_stage_variable_type::opaque_t;
}

namespace _detail {

template <bool storage_image = false>
struct ste_shader_stage_variable_type_from_type_impl {
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<std::is_array_v<V>>* = nullptr) {
		return variable_type_impl<std::remove_extent_t<V>>();
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<is_block_layout_v<V>>* = nullptr) {
		return ste_shader_stage_variable_type::struct_t;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<std::is_convertible_v<V, sampler>>* = nullptr) {
		return ste_shader_stage_variable_type::sampler_t;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<std::is_convertible_v<V, texture_generic>>* = nullptr) {
		return ste_shader_stage_variable_type::texture_t;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<std::is_convertible_v<V, image_view_generic>>* = nullptr) {
		return storage_image ?
			ste_shader_stage_variable_type::storage_image_t :
			ste_shader_stage_variable_type::image_t;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<is_arithmetic_v<V>>* = nullptr) {
		if (!is_scalar_v<V>) {
			return variable_type_impl<remove_extents_t<V>>();
		}

		if (std::numeric_limits<V>::is_integer) {
			if (std::is_same_v<V, bool>)
				return ste_shader_stage_variable_type::bool_t;
			return is_signed_v<V> ?
				ste_shader_stage_variable_type::int_t :
				ste_shader_stage_variable_type::uint_t;
		}
		if (is_floating_point_v<V>)
			return ste_shader_stage_variable_type::float_t;

		assert(false);
		return ste_shader_stage_variable_type::unknown;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(std::enable_if_t<std::is_void_v<V>>* = nullptr) {
		return ste_shader_stage_variable_type::void_t;
	}
	template <typename V>
	static constexpr ste_shader_stage_variable_type variable_type_impl(
		std::enable_if_t<std::negation_v<
			std::disjunction<
				std::is_array<V>,
				is_block_layout<V>,
				std::is_convertible<V, sampler>,
				std::is_convertible<V, texture_generic>,
				std::is_convertible<V, image_view_generic>,
				is_arithmetic<V>,
				std::is_void<V>
			>
		>>* = nullptr
	) {
		return ste_shader_stage_variable_type::unknown;
	}
};

}

template <typename T, bool storage_image = false>
struct ste_shader_stage_variable_type_from_type {
	static constexpr ste_shader_stage_variable_type value = 
		_detail::ste_shader_stage_variable_type_from_type_impl<storage_image>::template variable_type_impl<T>();
};
template <typename T, bool storage_image = false>
static constexpr auto ste_shader_stage_variable_type_from_type_v = ste_shader_stage_variable_type_from_type<T, storage_image>::value;

}
}
