// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "spectrum.hpp"

namespace StE {
namespace Graphics {

class RGB;
class xyY;

class XYZ : public spectrum<float, 3> {
public:
	using T = float;

public:
	using spectrum<float, 3>::spectrum;
	XYZ() = default;
	XYZ(const glm::tvec3<T> &vec) {
		X() = vec.r;
		Y() = vec.g;
		Z() = vec.b;
	}

	T &X() { return c[0]; }
	T &Y() { return c[1]; }
	T &Z() { return c[2]; }
	const T &X() const { return c[0]; }
	const T &Y() const { return c[1]; }
	const T &Z() const { return c[2]; }

	RGB toRGB() const;
	xyY to_xyY() const;

	T luminance() const override final { return Y(); };

	operator glm::tvec3<T>() const { return glm::tvec3<T>{ X(), Y(), Z() }; }
	operator RGB() const;
	operator xyY() const;
};

}
}

#include "RGB.hpp"
#include "xyY.hpp"

namespace StE {
namespace Graphics {

inline RGB XYZ::toRGB() const {
	RGB ret;
	ret.R() = 3.240479*X() - 1.537150*Y() - 0.498535*Z();
	ret.G() =-0.969256*X() + 1.875991*Y() + 0.041556*Z();
	ret.B() = 0.055648*X() - 0.204043*Y() + 1.057311*Z();
	return ret;
}

inline xyY XYZ::to_xyY() const {
	XYZ::T t = c[0] + c[1] + c[2];
	glm::tvec3<xyY::T> v;
	v.x = X() / t;
	v.y = Y() / t;
	v.z = Y();
	return v;
}

inline XYZ::operator RGB() const {
	return toRGB();
};

inline XYZ::operator xyY() const {
	return to_xyY();
};

}
}
