// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "entity.hpp"
#include "light_descriptor.hpp"

#include "observable_resource.hpp"

#include "texture_handle.hpp"
#include "rgb.hpp"

namespace StE {
namespace Graphics {

class light : public Core::observable_resource<light_descriptor>,
			  public entity {
	using Base = Core::observable_resource<light_descriptor>;

protected:
	light_descriptor descriptor;

public:
	light(float luminance, float radius, const rgb &diffuse) {
		descriptor.luminance = luminance;
		descriptor.radius = radius;
		descriptor.diffuse = decltype(descriptor.diffuse){ diffuse.R(), diffuse.G(), diffuse.B() };
	}
	virtual ~light() noexcept {};

	void set_luminance(float l) {
		descriptor.luminance = l;
		Base::notify();
	}
	void set_diffuse(const rgb &d) {
		descriptor.diffuse = decltype(descriptor.diffuse){ d.R(), d.G(), d.B() };
		Base::notify();
	}

	float get_luminance() const { return descriptor.luminance; }
	float get_radius() const { return descriptor.radius; }
	auto& get_diffuse() const { return descriptor.diffuse; }

	const light_descriptor& get_descriptor() const override final { return descriptor; }
};

}
}
