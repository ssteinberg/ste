// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "shaped_light.hpp"

namespace StE {
namespace Graphics {

class sphere_light : public shaped_light {
	using Base = shaped_light;

public:
	sphere_light(float luminance, const rgb &diffuse, const glm::vec3 &position, float radius) : shaped_light(LightType::Sphere,
																											  luminance, 
																											  diffuse,
																											  position,
																											  radius) {}
	virtual ~sphere_light() noexcept {}

	using Base::set_radius;
};

}
}
