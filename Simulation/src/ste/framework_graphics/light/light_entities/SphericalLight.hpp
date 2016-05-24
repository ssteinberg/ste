// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class SphericalLight : public light {
	using Base = light;

public:
	SphericalLight(float luminance, const RGB &diffuse, const glm::vec3 &position, float radius) : light(luminance, radius, diffuse) {
		descriptor.type = LightType::Sphere;
		descriptor.position = decltype(descriptor.position){ position.x, position.y, position.z };
	}
	virtual ~SphericalLight() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position = decltype(descriptor.position){ p.x, p.y, p.z };
		Base::notify();
	}
	void set_radius(float r) {
		descriptor.radius = r;
		Base::notify();
	}

	glm::vec3 get_position() const override { return { descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
};

}
}
