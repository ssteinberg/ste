// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <glm/gtx/dual_quaternion.hpp>

namespace StE {
namespace Graphics {

struct mesh_descriptor {
	glm::dualquat model_transform;

	glm::vec4 bounding_sphere;

	std::int32_t mat_idx;
	std::int32_t light_caster;

	float _unused[2];
};

struct mesh_draw_params {
	std::uint32_t count;
	std::uint32_t first_index;
	std::uint32_t base_vertex;

	float _unused;
};

}
}
