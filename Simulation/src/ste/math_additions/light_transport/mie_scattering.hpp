// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {

/*
*	The Cornette-Shanks Mie scattering phase function, an updated Henyey-Greenstein phase function.
*
*	@param cosine	Cosine of input angle.
*	@param g		Scattering variation parameter.
*					Ranges from backscattering (<0) through isotropic scattering (0) to forward scattering (>0).  (-1,+1)
*/
template <typename T>
T cornette_shanks_phase_function(const T &cosine, const T &g) {
	auto g2 = g*g;
	auto denom = static_cast<T>(8) * glm::pi<T>() * (static_cast<T>(2) + g2) * 
		glm::pow(static_cast<T>(1) + g2 - static_cast<T>(2) * g * cosine, 
				 static_cast<T>(3) / static_cast<T>(2));

	return static_cast<T>(3) * (static_cast<T>(1) + cosine*cosine) * (static_cast<T>(1) - g2) / denom;
}
/*
*	The Cornette-Shanks Mie scattering phase function, an updated Henyey-Greenstein phase function.
*
*	@param u		Normalized direction vector
*	@param v		Normalized direction vector
*	@param g		Scattering variation parameter.
*					Ranges from backscattering (<0) through isotropic scattering (0) to forward scattering (>0).  (-1,+1)
*/
template <typename T>
T cornette_shanks_phase_function(const glm::tvec3<T> &u, const glm::tvec3<T> &v, const T &g) {
	return cornette_shanks_phase_function(glm::dot(u, v), g);
}

}
