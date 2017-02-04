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

protected:
	light_descriptor descriptor;

protected:
	light(const rgb &color, float intensity, float radius) {
		auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.radius = radius;
		descriptor.emittance = c * intensity;
	}

public:
	virtual ~light() noexcept {};

	void set_luminance(const rgb &color, float intensity = 1.f) {
		auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.emittance = c * intensity;
		Base::notify();
	}

	auto &get_luminance() const { return descriptor.emittance; }
	float get_radius() const { return descriptor.radius; }

	const light_descriptor& get_descriptor() const override final { return descriptor; }
};

}
}
