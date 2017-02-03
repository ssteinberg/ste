// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "light_type.hpp"

namespace StE {
namespace Graphics {

struct light_descriptor {
	glm::vec3 position;	float radius{ .0f };
	glm::vec3 diffuse;	float luminance{ .0f };

	LightType type;

	float directional_distance;

	std::uint32_t cascade_idx{ 0xFFFFFFFF };

	float _internal[5];
};

}
}
