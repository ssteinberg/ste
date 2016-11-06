// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "observable_resource.hpp"

namespace StE {
namespace Graphics {

constexpr std::size_t directional_light_cascades = 6;

struct light_cascade_descriptor {
	glm::vec4 X, Y;
	glm::vec4 cascades_data[directional_light_cascades];
};

}
}
