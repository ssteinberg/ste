// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "spectrum.hpp"

namespace StE {
namespace Graphics {

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

	operator float() const { return K(); }
	operator rgb() const;
};

}
}

#include "rgb.hpp"
