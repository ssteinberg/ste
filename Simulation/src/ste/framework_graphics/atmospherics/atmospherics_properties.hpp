// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "beer_lambert.hpp"
#include "rayleigh_scattering.hpp"
#include "mie_scattering.hpp"

#include "RGB.hpp"

#include <limits>
#include <functional>

namespace StE {
namespace Graphics {

/*
 *	Defines the properties of an atmosphere
 */
template <typename T>
struct atmospherics_properties {
	using Vec = glm::tvec3<T>;

	// Center of the atmosphere and distance from center to where the atmosphere starts. 
	// Can be thought of as planet center and radius.
	Vec center;
	T radius;

	// Wave length dependent scattering coefficients for the Rayleigh scattering theory (m^-1)
	// For Rayleigh scattering extinction = scattering, i.e. no absorption by atmosphere molecules is assumed. Absorption is handled for in the Mie aerosol scattering.
	Vec rayleigh_scattering_coefficient;
	// Scattering coefficient for the Mie scattering theory (m^-1)
	T mie_scattering_coefficient;
	// Absorption coefficient for the Mie scattering theory (m^-1)
	T mie_absorption_coefficient;

	// Phase coefficient for the Mie scattering phase function
	T phase;

	// Sea level athmospheric pressure, kPa
	T ro0;
	// Sea level temperature, Kelvin
	T T0;
	// Gravitation acceleration, m/s^2
	T g;
	// Ideal (universal) gas constant, J/(mol·K)
	T R;
	// Molar mass of the atmospheric air, kg/mol
	T M;
	// Molar mass of the aerosols in atmosphere, kg/mol
	T M_aerosols;


	/*
	*	Returns the height of a point
	*	
	*	@param P	point in world coordinates
	*/
	T height(const Vec &P) const {
		return glm::length(P - center);
	}

	/*
	*	Returns the scale height of the atmosphere air
	*/
	T scale_height() const {
		return R * T0 / (M * g);
	}

	/*
	*	Returns the scale height of the aerosols
	*/
	T scale_height_aerosols() const {
		return R * T0 / (M_aerosols * g);
	}


	/*
	*	Returns the Mie extinction coefficient in m^-1
	*/
	T mie_extinction_coeffcient() const {
		return mie_scattering_coefficient + mie_absorption_coefficient;
	}

	/*
	*	Returns the Rayleigh extinction coefficient in m^-1
	*/
	Vec rayleigh_extinction_coeffcient() const {
		return rayleigh_scattering_coefficient;
	}


	/*
	*	Returns the height at which atmospheric density becomes negligible
	*/
	static T max_height(const T &H) {
		constexpr T max_density = 1e-6;

		auto log_density = glm::log(max_density);
		return -H * log_density;
	}

	/*
	*	Returns the pressure at h using the exponential barometric law
	*
	*	@param h	Height in meters
	*/
	T pressure(const T &h, const T &H) const {
		return ro0 * glm::exp(-h / H);
	}
};

/*
*	Creates an atmospherics_properties<double> object with properties resembling earth's atmosphere.
*
*	@param center	Atmosphere/planet center in world coordinates
*/
auto inline atmospherics_earth_properties(const glm::dvec3 &center) {
	atmospherics_properties<double> ap;

	ap.center = center;
	ap.radius = 6.371e+6;	// m

	ap.ro0 = 101.325;		// kPa
	ap.T0 = 288.15;			// K
	ap.g = 9.80665;			// m/s^2
	ap.R = 8.31447;			// J/(mol·K)
	ap.M = 0.0289644;		// kg/mol
	ap.M_aerosols = 0.203;	// kg/mol

	ap.phase = -.8;
	ap.rayleigh_scattering_coefficient = glm::dvec3{ 5.7, 13.36, 32.77 } * 1e-8;
	ap.mie_scattering_coefficient = 1.5e-7;
	ap.mie_absorption_coefficient = 2.5e-8;

	return ap;
}

}
}
