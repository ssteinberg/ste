// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "VertexBufferDescriptor.hpp"

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
};

}
}
