// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "spectrum.hpp"

namespace StE {
namespace Graphics {

class XYZ;

class RGB : public spectrum<float, 3> {
public:
	using T = float;

public:
	using spectrum<float, 3>::spectrum;
	RGB() = default;
	RGB(const glm::tvec3<T> &vec) {
		R() = vec.r;
		G() = vec.g;
		B() = vec.b;
	}
	RGB(const T &r, const T &g, const T &b) {
		R() = r;
		G() = g;
		B() = b;
	}

	T &R() { return c[0]; }
	T &G() { return c[1]; }
	T &B() { return c[2]; }
	const T &R() const { return c[0]; }
	const T &G() const { return c[1]; }
	const T &B() const { return c[2]; }

	XYZ toXYZ() const;

	T luminance() const override final;

	operator glm::tvec3<T>() const { return glm::tvec3<T>{ R(), G(), B() }; }
	operator XYZ() const;
};

}
}

#include "XYZ.hpp"

namespace StE {
namespace Graphics {

inline XYZ RGB::toXYZ() const {
	XYZ ret;
	ret.X() = 0.412453*R() + 0.357580*G() + 0.180423*B();
	ret.Y() = 0.212671*R() + 0.715160*G() + 0.072169*B();
	ret.Z() = 0.019334*R() + 0.119193*G() + 0.950227*B();
	return ret;
}

inline RGB::T RGB::luminance() const {
	return toXYZ().Y();
}

inline RGB::operator XYZ() const {
	return toXYZ();
};

}
}
