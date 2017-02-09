// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class virtual_light : public light {
	using Base = light;

public:
	virtual_light(const rgb &color,
				float intensity, 
				const glm::vec3 &position, 
				float radius) : light(color, intensity, radius) {
		descriptor.type = LightType::Point;
		descriptor.position = decltype(descriptor.position){ position.x, position.y, position.z };
	}
	virtual ~virtual_light() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position = decltype(descriptor.position){ p.x, p.y, p.z };
		Base::notify();
	}
	void set_radius(float r) {
		descriptor.radius = r;
		update_effective_range();
		Base::notify();
	}

	glm::vec3 get_position() const override { return { descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
};

}
}
