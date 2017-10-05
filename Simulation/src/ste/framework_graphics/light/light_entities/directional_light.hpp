// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <light.hpp>

namespace ste {
namespace graphics {

class directional_light : public light {
	using Base = light;

public:
	directional_light(const rgb &color,
					  cd_t intensity,
					  metre distance, 
					  metre radius, 
					  const glm::vec3 &direction) : light(color, intensity, radius) {
		descriptor.type = light_type::Direction;
		descriptor.position = decltype(descriptor.position){ metre(direction.x), metre(direction.y), metre(direction.z) };
		descriptor.effective_range_or_directional_distance = distance;
		descriptor.polygonal_light_points_and_offset_or_cascade_idx = 0xFFFFFFFF;
	}
	virtual ~directional_light() noexcept {}

	void update_effective_range(float sqrtA) final override {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position = decltype(descriptor.position){ metre(d.x), metre(d.y), metre(d.z) };
		Base::notify();
	}
	void set_radius(metre r) {
		descriptor.radius = r;
		Base::notify();
	}
	void set_distance(metre d) {
		descriptor.effective_range_or_directional_distance = d;
		Base::notify();
	}

	void set_cascade_idx(std::uint32_t idx) {
		descriptor.polygonal_light_points_and_offset_or_cascade_idx = idx;
		Base::notify();
	}

	metre_vec3 get_position() const override {
		const auto inf = std::numeric_limits<glm::vec3::value_type>::infinity();
		return { metre(inf), metre(inf), metre(inf) };
	}
	glm::vec3 get_direction() const { return { descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
	auto get_distance() const { return descriptor.effective_range_or_directional_distance; }
};

}
}
