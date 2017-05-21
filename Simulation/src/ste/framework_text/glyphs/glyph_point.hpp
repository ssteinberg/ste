// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vertex_input_layout.hpp>

namespace ste {
namespace text {

struct glyph_point : gl::vertex_input_layout<glm::u32vec4> {
	auto& data() { return get<0>(); }
	auto& data() const { return get<0>(); }

	constexpr static float size_scale = 32.f;
	constexpr static float weight_scale = .25f;
	constexpr static float stroke_width_scale = .25f;
};

}
}
