// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "atmospherics_properties.hpp"

namespace StE {
namespace Graphics {

class atmospherics_descriptor {
public:
	using T = float;
	using Properties = atmospherics_properties<T>;

private:
	struct precomputed_data {
		T M_over_R;
		T gM_over_RL;
	};

private:
	Properties properties;
	precomputed_data data;

private:
	// Padding
	static constexpr int properties_elements = sizeof(Properties) / sizeof(T);
	static constexpr int precomputed_data_elements = sizeof(precomputed_data) / sizeof(T);
	static constexpr int padding_size = (4 - (properties_elements + precomputed_data_elements) % 4) % 4;
	T _ununsed[padding_size];

	static_assert((sizeof(Properties) / sizeof(T) + precomputed_data_elements + padding_size) % 4 == 0, "Incorrect padding");

protected:
	void set_properties(const Properties &p) {
		this->properties = p;
		data.M_over_R = p.M / p.R;
		data.gM_over_RL = p.g * data.M_over_R / p.L;
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
