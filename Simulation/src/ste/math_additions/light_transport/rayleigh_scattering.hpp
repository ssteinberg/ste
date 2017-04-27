// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {

/*
*	The Rayleigh (normalized) phase function.
*
*	@param cosine	Cosine of input angle.
*/
template <typename T>
T rayleigh_phase_function(const T &cosine) {
	return static_cast<T>(3) / (static_cast<T>(16) * glm::pi<T>()) * (static_cast<T>(1) + cosine * cosine);
}
/*
*	The Rayleigh (normalized) phase function.
*
*	@param u	Normalized direction vector
*	@param v	Normalized direction vector
*/
template <typename T>
T rayleigh_phase_function(const glm::tvec3<T> &u, const glm::tvec3<T> &v) {
	return rayleigh_phase_function(glm::dot(u, v));
}

}
