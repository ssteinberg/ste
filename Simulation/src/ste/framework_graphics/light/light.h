// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "RGB.h"

#include "entity.h"

namespace StE {
namespace Graphics {

class light : public entity {
public:
	enum class LightType : std::int32_t {
		Sphere = 0,
		Directional = 1
	};

	struct light_descriptor {
		glm::vec4 position_direction;
		glm::vec4 diffuse;

		float luminance;
		float radius;

		LightType type;
	};

protected:
	bool dirty{ false };
	light_descriptor descriptor;

public:
	light(float luminance, const RGB &diffuse) {
		descriptor.luminance = luminance;
		descriptor.diffuse = decltype(descriptor.diffuse){ diffuse.R(), diffuse.G(), diffuse.B(), descriptor.diffuse.w };
	}
	virtual ~light() noexcept {};

	bool is_dirty() const { return dirty; }
	void clear_dirty() { dirty = false; }

	void set_luminance(float l) {
		descriptor.luminance = l;
		dirty = true;
	}
	void set_diffuse(const RGB &d) {
		descriptor.diffuse = decltype(descriptor.diffuse){ d.R(), d.G(), d.B(), descriptor.diffuse.w };
		dirty = true;
	}

	float get_luminance() const { return descriptor.luminance; }
	glm::vec3 get_diffuse() const { return { descriptor.diffuse.x, descriptor.diffuse.y, descriptor.diffuse.z }; }

	auto get_descriptor() const { return descriptor; }
};

}
}
