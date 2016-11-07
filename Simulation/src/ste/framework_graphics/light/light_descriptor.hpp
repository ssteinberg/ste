// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Graphics {

enum class LightType : std::int32_t {
	Sphere = 0,
	Directional = 1,
};

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
