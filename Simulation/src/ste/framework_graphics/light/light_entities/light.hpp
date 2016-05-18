// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "entity.hpp"

#include "texture_handle.hpp"
#include "RGB.hpp"

namespace StE {
namespace Graphics {

class light : public entity {
public:
	enum class LightType : std::int32_t {
		Sphere = 0,
		Directional = 1,
	};

	struct light_descriptor {
		glm::vec4 position_range;
		glm::vec3 diffuse;	float luminance;

		float radius;
		LightType type;

		float _reserved[2];
	};

protected:
	bool dirty{ false };
	light_descriptor descriptor;

public:
	light(float luminance, float radius, const RGB &diffuse) {
		descriptor.luminance = luminance;
		descriptor.radius = radius;
		descriptor.diffuse = decltype(descriptor.diffuse){ diffuse.R(), diffuse.G(), diffuse.B() };
	}
	virtual ~light() noexcept {};

	bool is_dirty() const { return dirty; }
	void clear_dirty() { dirty = false; }

	void set_luminance(float l) {
		descriptor.luminance = l;
		dirty = true;
	}
	void set_diffuse(const RGB &d) {
		descriptor.diffuse = decltype(descriptor.diffuse){ d.R(), d.G(), d.B() };
		dirty = true;
	}

	float get_luminance() const { return descriptor.luminance; }
	float get_radius() const { return descriptor.radius; }
	auto& get_diffuse() const { return descriptor.diffuse; }

	auto& get_descriptor() const { return descriptor; }
};

}
}
