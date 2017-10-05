//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <spectrum.hpp>

namespace ste {
namespace graphics {

class rgb;

class kelvin {
public:
	using T = float;

private:
	T t;

public:
	kelvin() = default;
	kelvin(float temperature) {
		K() = temperature;
	}

	T &K() { return t; }
	const T &K() const { return t; }

	rgb toRGB() const;

	operator rgb() const;
};

}


inline auto operator"" _K(unsigned long long int val) { return graphics::kelvin(static_cast<graphics::kelvin::T>(val)); }
inline auto operator"" _K(long double val) { return graphics::kelvin(static_cast<graphics::kelvin::T>(val)); }

}

#include <rgb.hpp>
