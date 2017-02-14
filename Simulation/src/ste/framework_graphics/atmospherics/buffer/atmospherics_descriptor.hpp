// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <atmospherics_properties.hpp>

namespace StE {
namespace Graphics {

namespace _detail {

template <typename T, int padding_size>
struct atmospherics_descriptor_padder { T pad[padding_size]; };
template <typename T>
struct atmospherics_descriptor_padder<T, 0> {};

}

class atmospherics_descriptor {
public:
	using Properties = atmospherics_properties<double>;

private:
	struct descriptor_data {
		glm::vec4 center_radius;

		glm::vec4 scattering_coefficients;
		float mie_absorption_coefficient;
		float phase;

		float Hm;
		float Hr;

		float minus_one_over_Hm;
		float minus_one_over_Hr;

		float Hm_max;
		float Hr_max;
	};

private:
	descriptor_data data;

private:
	// Padding
	static constexpr int properties_elements = sizeof(descriptor_data) / sizeof(float);
	static constexpr int padding_size = (4 - (properties_elements) % 4) % 4;
	_detail::atmospherics_descriptor_padder<float, padding_size> padder;

	static_assert((sizeof(descriptor_data) / sizeof(float) + padding_size) % 4 == 0, "Incorrect padding");

protected:
	void set_properties(const Properties &p) {
		data.center_radius = { p.center.x, p.center.y, p.center.z, p.radius };

		auto scatter = p.ro0 * glm::dvec4{
			p.rayleigh_scattering_coefficient.x,
			p.rayleigh_scattering_coefficient.y,
			p.rayleigh_scattering_coefficient.z,
			p.mie_scattering_coefficient };
		data.scattering_coefficients = scatter;
		data.mie_absorption_coefficient = p.ro0 * p.mie_absorption_coefficient;
		data.phase = p.phase;

		data.Hm = p.scale_height_aerosols();
		data.Hr = p.scale_height();

		data.minus_one_over_Hm = -1.f / data.Hm;
		data.minus_one_over_Hr = -1.f / data.Hr;

		data.Hm_max = p.max_height(data.Hm);
		data.Hr_max = p.max_height(data.Hr);
	}

public:
	atmospherics_descriptor(atmospherics_descriptor &&) = default;
	atmospherics_descriptor(const atmospherics_descriptor &) = default;
	atmospherics_descriptor &operator=(atmospherics_descriptor &&) = default;
	atmospherics_descriptor &operator=(const atmospherics_descriptor &) = default;

	atmospherics_descriptor(const Properties &p) {
		set_properties(p);
	}
	atmospherics_descriptor &operator=(const Properties &p) {
		set_properties(p);
		return *this;
	}
};

}
}
