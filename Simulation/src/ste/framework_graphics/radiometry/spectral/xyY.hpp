// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <spectrum.hpp>

namespace ste {
namespace graphics {

class XYZ;

class xyY : public spectrum<float, 3> {
public:
	using T = float;

public:
	using spectrum<float, 3>::spectrum;
	xyY() = default;
	xyY(const glm::tvec3<T> &vec) {
		x() = vec.r;
		y() = vec.g;
		Y() = vec.b;
	}

	T &x() { return c[0]; }
	T &y() { return c[1]; }
	T &Y() { return c[2]; }
	const T &x() const { return c[0]; }
	const T &y() const { return c[1]; }
	const T &Y() const { return c[2]; }

	XYZ toXYZ() const;

	T luminance() const override final { return Y(); };

	operator glm::tvec3<T>() const { return glm::tvec3<T>{ x(), y(), Y() }; }
	operator XYZ() const;
};

}
}

#include <XYZ.hpp>

namespace ste {
namespace graphics {

inline XYZ xyY::toXYZ() const {
	XYZ ret;
	float Y_y = Y() / y();
	ret.X() = Y_y * x();
	ret.Z() = Y_y * (1 - x() - y());
	ret.Y() = Y();
	return ret;
}

inline xyY::operator XYZ() const {
	return toXYZ();
};

}
}
