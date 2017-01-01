// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <limits>

namespace StE {
namespace Graphics {

/*
 *	Defines the properties of an atmosphere
 */
template <typename T>
struct atmospherics_properties {
	using Vec = glm::tvec3<T>;

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
	*	Returns the pressure at h using the exponential barometric law
	*
	*	@param h	Height in meters
	*/
	T pressure(const T &h) const {
		auto H = scale_height();
		return ro0 * glm::exp(-h / H);
	}

	/*
	*	Returns the aerosols pressure at h using the exponential barometric law
	*
	*	@param h	Height in meters
	*/
	T pressure_aerosols(const T &h) const {
		auto H = scale_height_aerosols();
		return ro0 * glm::exp(-h / H);
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
	*	Returns the optical length between 2 points.
	*	Optical length expresses the amount of light attenuated from point P0 to P1.
	*	Computed using an analytical solution to the integral of the barometric law from P0 to P1.
	*
	*	@param P0	Start point
	*	@param P1	End point
	*	@param H	Scale height
	*/
	T optical_length(const Vec &P0, const Vec &P1, const T &H) const {
		//! Currently the planet is flat...
		T h0 = glm::max(P0.y, static_cast<T>(0));
		T h1 = glm::max(P1.y, static_cast<T>(0));

		T len = glm::length(P1 - P0);
		T climb = glm::abs(h1 - h0);

		if (climb < static_cast<T>(1e-2))
			return len * pressure(glm::mix(h0, h1, static_cast<T>(.5)));
		else
			return len / climb * ro0 * H *
					glm::abs(glm::exp(-h0 / H) - glm::exp(-h1 / H));
	}

	/*
	*	Returns the optical length of a light ray originating at infinite height in direction
	*	V and ending at P1.
	*	For more details see optical_length().
	*
	*	@param P1	End point
	*	@param V	Normalized ray direction
	*	@param H	Scale height
	*/
	T optical_length_from_infinity(const Vec &P1, const Vec &V, const T &H) const {
		Vec up = { 0, 1, 0 };

		//! Currently the planet is flat...
		T h1 = glm::max(P1.y, static_cast<T>(0));

		T delta = glm::dot(up, -V);
		// Ignore rays coming from below the ground
		if (delta <= 0)
			return +std::numeric_limits<T>::infinity();

		T normalizer = static_cast<T>(1) / delta;

		return normalizer * ro0 * H * glm::exp(-h1 / H);
	}
};

auto inline atmospherics_earth_properties() {
	atmospherics_properties<double> ap;

	ap.ro0 = 101.325;		// kPa
	ap.T0 = 288.15;			// K
	ap.g = 9.80665;			// m/s^2
	ap.R = 8.31447;			// J/(mol·K)
	ap.M = 0.0289644;		// kg/mol
	ap.M_aerosols = 0.3;	// kg/mol

	ap.phase = -.88;
	ap.rayleigh_scattering_coefficient = glm::dvec3{ 5.7, 13.36, 32.77 } * 1e-8;
	ap.mie_scattering_coefficient = 2e-7;
	ap.mie_absorption_coefficient = 2.2e-8;

	return ap;
}

}
}
