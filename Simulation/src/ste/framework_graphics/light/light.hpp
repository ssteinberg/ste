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
private:
	static constexpr float light_cutoff = 0.001f;

public:
	enum class LightType : std::int32_t {
		Sphere = 0,
		Directional = 1,
	};

	struct light_descriptor {
		glm::vec4 position_direction;
		glm::vec4 diffuse;

		float luminance;
		float radius;
		float effective_range;

		LightType type;
	};

protected:
	bool dirty{ false };
	light_descriptor descriptor;

	static float calculate_effective_range(float lum, float r) {
		return r * (glm::sqrt(lum / light_cutoff) - 1.f);
	}

public:
	light(float luminance, float radius, const RGB &diffuse) {
		descriptor.luminance = luminance;
		descriptor.radius = radius;
		descriptor.effective_range = calculate_effective_range(luminance, radius);
		descriptor.diffuse = decltype(descriptor.diffuse){ diffuse.R(), diffuse.G(), diffuse.B(), descriptor.diffuse.w };
	}
	virtual ~light() noexcept {};

	bool is_dirty() const { return dirty; }
	void clear_dirty() { dirty = false; }

	void set_radius(float r) {
		descriptor.radius = r;
		descriptor.effective_range = calculate_effective_range(descriptor.luminance, descriptor.radius);
		dirty = true;
	}
	void set_luminance(float l) {
		descriptor.luminance = l;
		descriptor.effective_range = calculate_effective_range(descriptor.luminance, descriptor.radius);
		dirty = true;
	}
	void set_diffuse(const RGB &d) {
		descriptor.diffuse = decltype(descriptor.diffuse){ d.R(), d.G(), d.B(), descriptor.diffuse.w };
		dirty = true;
	}

	float get_luminance() const { return descriptor.luminance; }
	float get_radius() const { return descriptor.radius; }
	float get_effective_range() const { return descriptor.effective_range; }
	glm::vec3 get_diffuse() const { return { descriptor.diffuse.x, descriptor.diffuse.y, descriptor.diffuse.z }; }

	auto get_descriptor() const { return descriptor; }
};

}
}
