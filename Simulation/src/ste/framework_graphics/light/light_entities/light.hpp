// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <entity.hpp>

#include <light_type.hpp>
#include <light_descriptor.hpp>

#include <observable_resource.hpp>

#include <rgb.hpp>

namespace ste {
namespace graphics {

class light : public gl::observable_resource<light_descriptor::buffer_data>,
			  public entity {
	using Base = gl::observable_resource<light_descriptor::buffer_data>;

	static constexpr float light_minimal_luminance_multiplier = 1e-6f;

protected:
	light_descriptor descriptor;

private:
	float sqrt_surface_area;

protected:
	light(const rgb &color, 
		  cd_t intensity, 
		  metre radius) {
		const  auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.radius = radius;
		descriptor.emittance = c * static_cast<float>(intensity);

		sqrt_surface_area = static_cast<float>(radius) * glm::sqrt(glm::pi<float>());
		update_effective_range();
	}

	virtual void update_effective_range(float sqrtA) {
		sqrt_surface_area = sqrtA;

		rgb emittance = descriptor.emittance;
		const float I0 = emittance.luminance();
		const float I = light_minimal_luminance_multiplier * I0;

		descriptor.effective_range_or_directional_distance = metre(sqrtA * glm::sqrt(I0 / I));
	}
	void update_effective_range() {
		update_effective_range(sqrt_surface_area);
	}

public:
	virtual ~light() noexcept {};

	void set_luminance(const rgb &color, 
					   cd_t intensity) {
		const auto lum = color.luminance();
		auto c = static_cast<decltype(descriptor.emittance)>(color);
		c = lum > 0 ? c / lum : c;

		descriptor.emittance = c * static_cast<float>(intensity);
		update_effective_range();
		Base::notify();
	}

	auto &get_luminance() const { return descriptor.emittance; }
	metre get_radius() const { return descriptor.radius; }

	light_descriptor::buffer_data get_descriptor() const override final { return descriptor.get(); }
};

}
}
