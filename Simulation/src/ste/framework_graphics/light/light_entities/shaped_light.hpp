// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

namespace StE {
namespace Graphics {

class shaped_light : public light {
	using Base = light;

protected:
	shaped_light(LightType type, float luminance, const rgb &diffuse, const glm::vec3 &position, float radius) : light(luminance, 
																													   radius, 
																													   diffuse) {
		assert(static_cast<std::uint32_t>(type) & light_type_shape_bit && "Type is not a shaped light!");

		descriptor.type = type;
		descriptor.position = decltype(descriptor.position){ position.x, position.y, position.z };
	}
	void set_radius(float r) {
		descriptor.radius = r;
		Base::notify();
	}

public:
	virtual ~shaped_light() noexcept {}

	void set_position(const glm::vec3 &p) {
		descriptor.position = decltype(descriptor.position){ p.x, p.y, p.z };
		Base::notify();
	}

	glm::vec3 get_position() const override { return{ descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
};

}
}
