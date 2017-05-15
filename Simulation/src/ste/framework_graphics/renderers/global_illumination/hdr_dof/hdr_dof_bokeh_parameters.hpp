// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

namespace ste {
namespace graphics {

struct hdr_bokeh_parameters : gl::std430<std::int32_t, std::int32_t, float> {
	using Base = gl::std430<std::int32_t, std::int32_t, float>;
	using Base::Base;

	auto& lum_min() { return get<0>(); }
	auto& lum_max() { return get<1>(); }
	auto& focus() { return get<2>(); }
};

}
}
