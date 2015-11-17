// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "RGB.h"

namespace StE {
namespace Graphics {

class light {
public:
	enum class LightType : std::uint32_t {
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
		descriptor.diffuse.xyz = diffuse;
	}
	virtual ~light() noexcept {};

	bool is_dirty() const { return dirty; }
	void clear_dirty() { dirty = false; }

	void set_luminance(float l) {
		descriptor.luminance = l;
		dirty = true;
	}
	void set_diffuse(const RGB &d) {
		descriptor.diffuse.xyz = d;
		dirty = true;
	}

	float get_luminance() const { return descriptor.luminance; }
	glm::vec3 get_diffuse() const { return descriptor.diffuse.xyz; }

	auto get_descriptor() const { return descriptor; }
};

}
}
