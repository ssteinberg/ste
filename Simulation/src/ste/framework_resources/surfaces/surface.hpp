//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>

#include <surface_impl.hpp>
#include <surface_array.hpp>
#include <surface_cubemap.hpp>

#include <type_traits>

namespace ste {
namespace resource {

template<gl::format format, gl::image_type image_type>
using surface_generic = _detail::surface_base<format, image_type>;

template<gl::format format>
using surface_1d = _detail::surface<format, gl::image_type::image_1d>;
template<gl::format format>
using surface_1d_array = _detail::surface_array<format, gl::image_type::image_1d_array>;
template<gl::format format>
using surface_2d = _detail::surface<format, gl::image_type::image_2d>;
template<gl::format format>
using surface_2d_array = _detail::surface_array<format, gl::image_type::image_2d_array>;
template<gl::format format>
using surface_3d = _detail::surface<format, gl::image_type::image_3d>;
template<gl::format format>
using surface_cubemap = _detail::surface_cubemap<format>;
template<gl::format format>
using surface_cubemap_array = _detail::surface_array<format, gl::image_type::image_cubemap_array>;

template<gl::format format, gl::image_type image_type>
using surface = std::conditional_t<
	image_type == gl::image_type::image_1d, surface_1d<format>, std::conditional_t<
	image_type == gl::image_type::image_2d, surface_2d<format>, std::conditional_t<
	image_type == gl::image_type::image_3d, surface_3d<format>, std::conditional_t<
	image_type == gl::image_type::image_1d_array, surface_1d_array<format>, std::conditional_t<
	image_type == gl::image_type::image_2d_array, surface_2d_array<format>, std::conditional_t<
	image_type == gl::image_type::image_cubemap, surface_cubemap<format>, surface_cubemap_array<format>>>>>>>;

}
}
