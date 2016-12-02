// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "atmospherics_properties.hpp"

namespace StE {
namespace Graphics {

class atmospherics_descriptor : private atmospherics_properties<float> {
	using Base = atmospherics_properties<float>;

public:
	using Properties = Base;

private:
	float M_over_R;
	float gM_over_RL;

protected:
	void set_properties(const Base &p) {
		Base::operator=(p);
		M_over_R = p.M / p.R;
		gM_over_RL = p.g * M_over_R / p.L;
	}

public:
	atmospherics_descriptor(atmospherics_descriptor &&) = default;
	atmospherics_descriptor(const atmospherics_descriptor &) = default;
	atmospherics_descriptor &operator=(atmospherics_descriptor &&) = default;
	atmospherics_descriptor &operator=(const atmospherics_descriptor &) = default;

	atmospherics_descriptor(const Base &p) {
		set_properties(p);
	}
	atmospherics_descriptor &operator=(const Base &p) {
		set_properties(p);
		return *this;
	}
};

}
}
