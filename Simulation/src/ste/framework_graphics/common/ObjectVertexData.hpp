// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "VertexBufferDescriptor.hpp"

namespace StE {
namespace Graphics {

struct ObjectVertexData {
	glm::vec3 p, n, t;
	glm::vec2 uv;

	using descriptor = LLR::VBODescriptorWithTypes<glm::vec3, glm::vec3, glm::vec3, glm::vec2>::descriptor;
};

}
}
