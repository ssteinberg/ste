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
	constexpr kelvin() = default;
	constexpr kelvin(T temperature) : t(temperature) {}

	constexpr T &K() { return t; }
	constexpr const T &K() const { return t; }

	rgb toRGB() const;

	operator rgb() const;
};

}


constexpr auto operator"" _K(unsigned long long int val) { return graphics::kelvin(static_cast<graphics::kelvin::T>(val)); }
constexpr auto operator"" _K(long double val) { return graphics::kelvin(static_cast<graphics::kelvin::T>(val)); }

}

#include <rgb.hpp>
