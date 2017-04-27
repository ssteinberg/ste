// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace Graphics {

class mesh_aabb {
private:
	glm::vec3 va, vb;

public:
	mesh_aabb() = default;
	mesh_aabb(const glm::vec3 &va, const glm::vec3 &vb) : va(va), vb(vb) {}

	auto &a() { return va; }
	auto &b() { return vb; }
	const auto &a() const { return va; }
	const auto &b() const { return vb; }
};

}
}
