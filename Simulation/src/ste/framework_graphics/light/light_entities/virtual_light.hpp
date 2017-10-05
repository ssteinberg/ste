// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <light.hpp>

namespace ste {
namespace graphics {

class virtual_light : public light {
	using Base = light;

public:
	virtual_light(const rgb &color,
				  cd_t intensity,
				  const metre_vec3 &position,
				  metre radius) : light(color, intensity, radius) {
		descriptor.type = light_type::Point;
		descriptor.position = decltype(descriptor.position){ position.x, position.y, position.z };
	}

	virtual ~virtual_light() noexcept {}

	void set_position(const metre_vec3 &p) {
		descriptor.position = p;
		Base::notify();
	}

	void set_radius(metre r) {
		descriptor.radius = r;
		update_effective_range();
		Base::notify();
	}

	metre_vec3 get_position() const override { return descriptor.position; }
};

}
}
