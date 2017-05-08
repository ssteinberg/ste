// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <spectrum.hpp>

#include <limits>

namespace ste {
namespace graphics {

class rgb;
class xyY;
class kelvin;

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

	rgb toRGB() const;
	xyY to_xyY() const;
	kelvin toKelvin() const;

	T luminance() const override final { return Y(); };

	operator glm::tvec3<T>() const { return glm::tvec3<T>{ X(), Y(), Z() }; }
	operator rgb() const;
	operator xyY() const;
	operator kelvin() const;
};

}
}

#include <rgb.hpp>
#include <xyY.hpp>
#include <kelvin.hpp>

namespace ste {
namespace graphics {

inline rgb XYZ::toRGB() const {
	rgb ret;
	ret.R() =  3.240479f * X() - 1.537150f * Y() - 0.498535f * Z();
	ret.G() = -0.969256f * X() + 1.875991f * Y() + 0.041556f * Z();
	ret.B() =  0.055648f * X() - 0.204043f * Y() + 1.057311f * Z();
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

inline XYZ::operator rgb() const {
	return toRGB();
};

inline XYZ::operator xyY() const {
	return to_xyY();
};

inline XYZ::operator kelvin() const {
	return toKelvin();
};

}
}
