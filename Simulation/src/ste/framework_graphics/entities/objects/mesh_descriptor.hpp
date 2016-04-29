// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Graphics {

struct mesh_descriptor {
	glm::mat4 model;
	glm::mat4 transpose_inverse_model;

	glm::vec4 bounding_sphere;

	std::int32_t mat_idx;

	std::uint32_t count;
	std::uint32_t first_index;
	std::uint32_t base_vertex;
};

}
}
