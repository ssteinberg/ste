// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atmospherics_properties.hpp>
#include <std430.hpp>

namespace ste {
namespace graphics {

namespace _detail {

template <typename T, int padding_size>
struct atmospherics_descriptor_padder { alignas(1) T pad[padding_size]; };
template <typename T>
struct atmospherics_descriptor_padder<T, 0> {};

}

class atmospherics_descriptor {
public:
	using Properties = atmospherics_properties<double>;
	struct descriptor_data : gl::std430<glm::vec4, glm::vec4, float, float, float, float, float, float, float, float> {
		auto& center_radius() { return get<0>(); }

		auto& scattering_coefficients() { return get<1>(); }
		auto& mie_absorption_coefficient() { return get<2>(); }
		auto& phase() { return get<3>(); }

		auto& Hm() { return get<4>(); }
		auto& Hr() { return get<5>(); }

		auto& minus_one_over_Hm() { return get<6>(); }
		auto& minus_one_over_Hr() { return get<7>(); }

		auto& Hm_max() { return get<8>(); }
		auto& Hr_max() { return get<9>(); }
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
		data.center_radius() = { p.center.x, p.center.y, p.center.z, p.radius };

		auto scatter = p.ro0 * glm::dvec4{
			p.rayleigh_scattering_coefficient.x,
			p.rayleigh_scattering_coefficient.y,
			p.rayleigh_scattering_coefficient.z,
			p.mie_scattering_coefficient };
		data.scattering_coefficients() = scatter;
		data.mie_absorption_coefficient() = static_cast<float>(p.ro0 * p.mie_absorption_coefficient);
		data.phase() = static_cast<float>(p.phase);

		data.Hm() = static_cast<float>(p.scale_height_aerosols());
		data.Hr() = static_cast<float>(p.scale_height());

		data.minus_one_over_Hm() = static_cast<float>(-1.f / data.Hm());
		data.minus_one_over_Hr() = static_cast<float>(-1.f / data.Hr());

		data.Hm_max() = static_cast<float>(p.max_height(data.Hm()));
		data.Hr_max() = static_cast<float>(p.max_height(data.Hr()));
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

	const auto& get() const { return data; }
};

}
}
