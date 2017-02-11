// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "entity.hpp"

#include "light_type.hpp"
#include "light_descriptor.hpp"

#include "observable_resource.hpp"

#include "rgb.hpp"

namespace StE {
namespace Graphics {

class light : public Core::observable_resource<light_descriptor>,
			  public entity {
	using Base = Core::observable_resource<light_descriptor>;

	static constexpr float light_minimal_luminance_multiplier = 1e-6f;

protected:
	light_descriptor descriptor;

protected:
	light(const rgb &color, float intensity, float radius) {
		auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.radius = radius;
		descriptor.emittance = c * intensity;
		descriptor.sqrt_surface_area = radius;
		update_effective_range();
	}

	void update_effective_range() {
		rgb emittance = descriptor.emittance;
		float sqrtA = descriptor.sqrt_surface_area;
		float I0 = emittance.luminance();
		float I = light_minimal_luminance_multiplier * I0;

		descriptor.effective_range = sqrtA * glm::sqrt(I0 / I);
	}

public:
	virtual ~light() noexcept {};

	void set_luminance(const rgb &color, float intensity = 1.f) {
		auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.emittance = c * intensity;
		update_effective_range();
		Base::notify();
	}

	auto &get_luminance() const { return descriptor.emittance; }
	float get_radius() const { return descriptor.radius; }

	const light_descriptor& get_descriptor() const override final { return descriptor; }
};

}
}
