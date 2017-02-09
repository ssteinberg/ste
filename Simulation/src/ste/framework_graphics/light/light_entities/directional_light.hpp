// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class directional_light : public light {
	using Base = light;

public:
	directional_light(const rgb &color,
					  float intensity,
					  float distance, 
					  float radius, 
					  const glm::vec3 &direction) : light(color, intensity, radius) {
		descriptor.type = LightType::Direction;
		descriptor.position = decltype(descriptor.position){ direction.x, direction.y, direction.z };
		descriptor.directional_distance = distance;
	}
	virtual ~directional_light() noexcept {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position = decltype(descriptor.position){ d.x, d.y, d.z };
		Base::notify();
	}
	void set_radius(float r) {
		descriptor.radius = r;
		update_effective_range();
		Base::notify();
	}
	void set_distance(float d) {
		descriptor.directional_distance = d;
		Base::notify();
	}

	void set_cascade_idx(std::uint32_t idx) {
		descriptor.cascade_idx = idx;
		Base::notify();
	}

	glm::vec3 get_position() const override {
		auto inf = std::numeric_limits<glm::vec3::value_type>::infinity();
		return { inf, inf, inf };
	}
	glm::vec3 get_direction() const { return { descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
	auto get_distance() const { return descriptor.directional_distance; }
};

}
}
