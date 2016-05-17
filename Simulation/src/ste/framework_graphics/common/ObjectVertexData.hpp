// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "VertexBufferDescriptor.hpp"
#include "quaternion_tangent_frame.hpp"

namespace StE {
namespace Graphics {

class ObjectVertexData {
public:
	glm::quat tangent_frame_quat;
	glm::vec3 p;
	glm::vec2 uv;

public:
	using descriptor = Core::VBODescriptorWithTypes<glm::vec4, glm::vec3, glm::vec2>::descriptor;

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
