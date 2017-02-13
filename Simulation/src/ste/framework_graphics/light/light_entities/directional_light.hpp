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
		descriptor.effective_range_or_directional_distance = distance;
		descriptor.polygonal_light_points_and_offset_or_cascade_idx = 0xFFFFFFFF;
	}
	virtual ~directional_light() noexcept {}

	void update_effective_range(float sqrtA) final override {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position = decltype(descriptor.position){ d.x, d.y, d.z };
		Base::notify();
	}
	void set_radius(float r) {
		descriptor.radius = r;
		Base::notify();
	}
	void set_distance(float d) {
		descriptor.effective_range_or_directional_distance = d;
		Base::notify();
	}

	void set_cascade_idx(std::uint32_t idx) {
		descriptor.polygonal_light_points_and_offset_or_cascade_idx = idx;
		Base::notify();
	}

	glm::vec3 get_position() const override {
		auto inf = std::numeric_limits<glm::vec3::value_type>::infinity();
		return { inf, inf, inf };
	}
	glm::vec3 get_direction() const { return { descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
	auto get_distance() const { return descriptor.effective_range_or_directional_distance; }
};

}
}
