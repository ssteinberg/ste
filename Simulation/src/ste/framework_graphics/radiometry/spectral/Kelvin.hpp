// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <spectrum.hpp>

namespace ste {
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

	operator rgb() const;
};

}
}

#include <rgb.hpp>
