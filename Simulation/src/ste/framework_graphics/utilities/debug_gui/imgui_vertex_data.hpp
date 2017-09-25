// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <vertex_input_layout.hpp>

namespace ste {
namespace graphics {

struct imgui_vertex_data : gl::vertex_input_layout<glm::vec2, glm::vec2, std::uint32_t> {
	auto& position() { return get<0>(); }
	auto& uv() { return get<1>(); }
	auto& color() { return get<2>(); }

	auto& position() const { return get<0>(); }
	auto& uv() const { return get<1>(); }
	auto& color() const { return get<2>(); }
};

}
}
