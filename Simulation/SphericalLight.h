// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "light.h"

namespace StE {
namespace Graphics {

class SphericalLight : public light {
public:
	SphericalLight(float luminance, const RGB &diffuse, glm::vec3 position, float radius) : light(luminance, diffuse) {
		descriptor.type = LightType::Sphere;
		descriptor.position_direction.xyz = position;
		descriptor.radius = radius;
	}
	virtual ~SphericalLight() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position_direction.xyz = p;
		dirty = true;
	}
	void set_radius(float r) {
		descriptor.radius = r;
		dirty = true;
	}

	glm::vec3 get_position() const { return descriptor.position_direction.xyz; }
	float get_radius() const { return descriptor.radius; };
};

}
}
