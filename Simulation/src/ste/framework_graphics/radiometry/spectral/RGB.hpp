// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <spectrum.hpp>

namespace ste {
namespace Graphics {

class XYZ;

class rgb : public spectrum<float, 3> {
public:
	using T = float;

public:
	using spectrum<float, 3>::spectrum;
	rgb() = default;
	rgb(const glm::tvec3<T> &vec) {
		R() = vec.r;
		G() = vec.g;
		B() = vec.b;
	}
	rgb(const T &r, const T &g, const T &b) {
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

#include <XYZ.hpp>

namespace ste {
namespace Graphics {

inline XYZ rgb::toXYZ() const {
	XYZ ret;
	ret.X() = 0.412453f * R() + 0.357580f * G() + 0.180423f * B();
	ret.Y() = 0.212671f * R() + 0.715160f * G() + 0.072169f * B();
	ret.Z() = 0.019334f * R() + 0.119193f * G() + 0.950227f * B();
	return ret;
}

inline rgb::T rgb::luminance() const {
	return toXYZ().Y();
}

inline rgb::operator XYZ() const {
	return toXYZ();
};

}
}
