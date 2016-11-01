// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "observable_resource.hpp"

namespace StE {
namespace Graphics {

constexpr std::size_t directional_light_cascades = 6;

struct light_cascade_descriptor {
	struct cascade_data {
		glm::mat4 cascade_mat;
		glm::vec2 viewport_size;
		float cascade_depth;
		float _unused;
	};
	glm::vec4 X, Y;
	cascade_data cascade[directional_light_cascades];
};

}
}
