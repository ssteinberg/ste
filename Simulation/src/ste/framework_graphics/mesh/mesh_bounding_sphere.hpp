// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace Graphics {

class mesh_bounding_sphere {
private:
	glm::vec4 s;

public:
	mesh_bounding_sphere() = default;
	mesh_bounding_sphere(const glm::vec3 &c, float r) : s(c.x, c.y, c.z, r) {}

	auto &sphere() { return s; }
	auto &sphere() const { return s; }
};

}
}
