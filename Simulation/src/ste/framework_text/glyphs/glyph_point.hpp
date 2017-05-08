// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vertex_attributes.hpp>

namespace ste {
namespace text {

struct glyph_point {
	glm::u32vec4 data;

	constexpr static float size_scale = 32.f;
	constexpr static float weight_scale = .25f;
	constexpr static float stroke_width_scale = .25f;
	using descriptor = gl::vertex_attributes<glm::u32vec4>;

	glyph_point() = default;
};

}
}
