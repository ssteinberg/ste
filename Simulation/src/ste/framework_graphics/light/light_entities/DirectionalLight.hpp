// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class DirectionalLight : public light {
public:
	DirectionalLight(float luminance, const RGB &diffuse, const glm::vec3 &direction) : light(luminance, 1.f, diffuse) {
		descriptor.type = LightType::Directional;
		descriptor.position_range = decltype(descriptor.position_range){ direction.x, direction.y, direction.z, 0 };
	}
	virtual ~DirectionalLight() noexcept {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position_range = decltype(descriptor.position_range){ d.x, d.y, d.z, 0 };
		dirty = true;
	}

	glm::vec3 get_position() const override {
		auto inf = std::numeric_limits<glm::vec3::value_type>::infinity();
		return { inf, inf, inf };
	}
	glm::vec3 get_direction() const { return { descriptor.position_range.x, descriptor.position_range.y, descriptor.position_range.z }; }
};

}
}
