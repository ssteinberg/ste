// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "light.h"

namespace StE {
namespace Graphics {

class DirectionalLight : public light {
public:
	DirectionalLight(float luminance, const RGB &diffuse, glm::vec3 direction) : light(luminance, diffuse) {
		descriptor.type = LightType::Directional;
		descriptor.position_direction.xyz = direction;
	}
	virtual ~DirectionalLight() noexcept {}

	void set_direction(const glm::vec3 &d) {
		descriptor.position_direction.xyz = d;
		dirty = true;
	}

	glm::vec3 get_direction() const { return descriptor.position_direction.xyz; }
};

}
}
