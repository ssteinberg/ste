//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface.hpp>

namespace ste {
namespace resource {

template <typename T>
struct is_opaque_surface {
	static constexpr bool value = false;
};
template <int d>
struct is_opaque_surface<opaque_surface<d>> {
	static constexpr bool value = true;
};
template <typename T>
static constexpr auto is_opaque_surface_v = is_opaque_surface<T>::value;

template <typename T>
struct is_surface {
	static constexpr bool value = false;
};
template <gl::format f>
struct is_surface<surface_1d<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_2d<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_3d<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_1d_array<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_2d_array<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_cubemap<f>> {
	static constexpr bool value = true;
};
template <gl::format f>
struct is_surface<surface_cubemap_array<f>> {
	static constexpr bool value = true;
};
template <typename T>
static constexpr bool is_surface_v = is_surface<T>::value;

template <typename T, bool is_surface = is_surface_v<T>>
struct surface_format {
	static_assert(is_surface, "T must be a surface");
};
template <typename T>
struct surface_format<T, true> {
	static constexpr gl::format value = T::surface_format();
};
template <typename T>
static constexpr auto surface_format_v = surface_format<T>::value;

template <typename T, bool is_surface = is_surface_v<T>>
struct surface_image_type {
	static_assert(is_surface, "T must be a surface");
};
template <typename T>
struct surface_image_type<T, true> {
	static constexpr gl::image_type value = T::surface_image_type();
};
template <typename T>
static constexpr auto surface_image_type_v = surface_image_type<T>::value;

template <typename T, bool is_surface = is_surface_v<T>>
struct surface_dimensions {
	static_assert(is_surface, "T must be a surface");
};
template <typename T>
struct surface_dimensions<T, true> {
	static constexpr auto value = T::surface_dimensions();
};
template <typename T>
static constexpr auto surface_dimensions_v = surface_dimensions<T>::value;

template <gl::image_type image_type>
struct surface_for_image_type {};
template <>
struct surface_for_image_type<gl::image_type::image_1d> {
	template <gl::format format>
	using type = surface_1d<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_2d> {
	template <gl::format format>
	using type = surface_2d<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_3d> {
	template <gl::format format>
	using type = surface_3d<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_1d_array> {
	template <gl::format format>
	using type = surface_1d_array<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_2d_array> {
	template <gl::format format>
	using type = surface_2d_array<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_cubemap> {
	template <gl::format format>
	using type = surface_cubemap<format>;
};
template <>
struct surface_for_image_type<gl::image_type::image_cubemap_array> {
	template <gl::format format>
	using type = surface_cubemap_array<format>;
};
template <gl::image_type image_type, gl::format format>
using surface_for_image_type_t = typename surface_for_image_type<image_type>::template type<format>;

}
}
