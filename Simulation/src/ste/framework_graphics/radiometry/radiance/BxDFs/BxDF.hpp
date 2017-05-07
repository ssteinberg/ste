// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

template <typename T>
class BxDF {
public:
	/**
	*	@brief	Compute cosine of theta for given radiance vector
	*
	* 	@param v	Normalized outgoing vector
	*/
	static T cos_theta(const glm::tvec3<T> &v) { return v.z; }

	/**
	*	@brief	Compute sine squared of theta for given radiance vector
	*
	* 	@param v	Normalized outgoing vector
	*/
	static T sin2_theta(const glm::tvec3<T> &v) { return glm::max(static_cast<T>(0), static_cast<T>(1) - cos_theta(v)*cos_theta(v)); }

	/**
	*	@brief	Compute sine of theta for given radiance vector
	*
	* 	@param v	Normalized outgoing vector
	*/
	static T sin_theta(const glm::tvec3<T> &v) { return glm::sqrt(sin2_theta(v)); }

	/**
	*	@brief	Compute cosine of phi for given radiance vector
	*
	* 	@param v	Normalized outgoing vector
	*/
	static T cos_phi(const glm::tvec3<T> &v) {
		T s = sin_theta(v);
		if (s == static_cast<T>(0)) return static_cast<T>(1);
		return glm::clamp(v.x / s, static_cast<T>(-1), static_cast<T>(1));
	}

	/**
	*	@brief	Compute sine of phi for given radiance vector
	*
	* 	@param v	Normalized outgoing vector
	*/
	static T sin_phi(const glm::tvec3<T> &v) {
		T s = sin_theta(v);
		if (s == static_cast<T>(0)) return 0.f;
		return glm::clamp(v.y / s, static_cast<T>(-1), static_cast<T>(1));
	}

	/**
	*	@brief	Compute radiance vector (omega) for given theta and phi
	*	(spherical coordinates)
	*
	* 	@param theta	In radians
	* 	@param phi		In radians
	*/
	static glm::tvec3<T> omega(const T &theta, const T &phi) {
		auto z = glm::cos(theta);
		auto x = glm::cos(phi);
		auto y = glm::sin(phi);

		glm::tvec2<T> xy(x, y);
		xy *= glm::sqrt(static_cast<T>(1) - z*z);

		return{ xy.x, xy.y, z };
	}
};

}
}
