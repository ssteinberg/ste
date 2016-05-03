// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class SphericalLight : public light {
public:
	SphericalLight(float luminance, const RGB &diffuse, const glm::vec3 &position, float radius) : light(luminance, radius, diffuse) {
		descriptor.type = LightType::Sphere;
		descriptor.position_direction = decltype(descriptor.position_direction){ position.x, position.y, position.z };
	}
	virtual ~SphericalLight() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position_direction = decltype(descriptor.position_direction){ p.x, p.y, p.z };
		dirty = true;
	}
	void set_radius(float r) {
		descriptor.radius = r;
		dirty = true;
	}

	glm::vec3 get_position() const override { return { descriptor.position_direction.x, descriptor.position_direction.y, descriptor.position_direction.z }; }
};

}
}
