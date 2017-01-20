// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "vertex_buffer_descriptor.hpp"
#include "quaternion_tangent_frame.hpp"

namespace StE {
namespace Graphics {

class object_vertex_data {
public:
	glm::quat tangent_frame_quat;
	glm::vec3 p;
	glm::vec2 uv;

public:
	using descriptor = Core::vbo_descriptor_with_types<glm::vec4, glm::vec3, glm::vec2>::descriptor;

public:
	void tangent_frame_from_tbn(const glm::vec3 &t, const glm::vec3 &b, const glm::vec3 &n) {
		tangent_frame_quat = tbn_to_tangent_frame(t, b, n);
	}

	glm::mat3 extract_tangent_frame() const {
		return StE::extract_tangent_frame(glm::quat(), tangent_frame_quat);
	}
};

}
}
