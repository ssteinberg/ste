// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <vertex_input_layout.hpp>
#include <quaternion_tangent_frame.hpp>

namespace ste {
namespace graphics {

struct object_vertex_data : gl::vertex_input_layout<glm::quat, glm::vec3, glm::vec2> {
	auto& tangent_frame_quat() { return get<0>(); }
	auto& p() { return get<1>(); }
	auto& uv() { return get<2>(); }

	auto& tangent_frame_quat() const { return get<0>(); }
	auto& p() const { return get<1>(); }
	auto& uv() const { return get<2>(); }

	void tangent_frame_from_tbn(const glm::vec3 &t, const glm::vec3 &b, const glm::vec3 &n) {
		tangent_frame_quat() = tbn_to_tangent_frame(t, b, n);
	}

	glm::mat3 extract_tangent_frame() const {
		return ste::extract_tangent_frame(glm::quat(), tangent_frame_quat());
	}
};

}
}
