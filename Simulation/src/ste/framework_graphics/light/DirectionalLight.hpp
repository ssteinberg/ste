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
		descriptor.position_direction = decltype(descriptor.position_direction){ direction.x, direction.y, direction.z, descriptor.position_direction.w};
	}
	virtual ~DirectionalLight() noexcept {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position_direction = decltype(descriptor.position_direction){ d.x, d.y, d.z, descriptor.position_direction.w};
		dirty = true;
	}

	glm::vec3 get_position() const override {
		auto inf = std::numeric_limits<glm::vec3::value_type>::infinity();
		return { inf, inf, inf };
	}
	glm::vec3 get_direction() const { return { descriptor.position_direction.x, descriptor.position_direction.y, descriptor.position_direction.z }; }
};

}
}
