// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "light.h"

namespace StE {
namespace Graphics {

class SphericalLight : public light {
public:
	SphericalLight(float luminance, const RGB &diffuse, const glm::vec3 &position, float radius) : light(luminance, diffuse) {
		descriptor.type = LightType::Sphere;
		descriptor.position_direction = decltype(descriptor.position_direction){ position.x, position.y, position.z, descriptor.position_direction.w};
		descriptor.radius = radius;
	}
	virtual ~SphericalLight() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position_direction = decltype(descriptor.position_direction){ p.x, p.y, p.z, descriptor.position_direction.w};
		dirty = true;
	}
	void set_radius(float r) {
		descriptor.radius = r;
		dirty = true;
	}

	glm::vec3 get_position() const override { return { descriptor.position_direction.x, descriptor.position_direction.y, descriptor.position_direction.z }; }
	float get_radius() const { return descriptor.radius; };
};

}
}
