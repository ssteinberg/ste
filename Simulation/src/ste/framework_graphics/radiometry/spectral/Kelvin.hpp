// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "spectrum.hpp"

namespace StE {
namespace Graphics {

class RGB;

class Kelvin {
public:
	using T = float;

private:
	T t;

public:
	Kelvin() = default;
	Kelvin(float temperature) {
		K() = temperature;
	}

	T &K() { return t; }
	const T &K() const { return t; }

	RGB toRGB() const;

	operator float() const { return K(); }
	operator RGB() const;
};

}
}

#include "RGB.hpp"
