//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>

#include <surface_impl.hpp>
#include <surface_array.hpp>
#include <surface_cubemap.hpp>

namespace ste {
namespace resource {

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

}
}
