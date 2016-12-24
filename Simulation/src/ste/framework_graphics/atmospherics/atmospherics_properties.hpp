// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"


namespace StE {
namespace Graphics {

/*
 *	Defines the properties of an atmosphere
 */
template <typename T>
struct atmospherics_properties {
	// Wave length dependent scattering coefficients for the Rayleigh scattering theory (m^-1)
	// For Rayleigh scattering extinction = scattering, i.e. no absorption by atmosphere molecules is assumed. Absorption is handled for in the Mie aerosol scattering.
	glm::tvec3<T> rayleigh_scattering_coefficient;
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
	// Temperature lapse rate, https://en.wikipedia.org/wiki/Adiabatic_lapse_rate
	// K/m
	T L;
	// Ideal (universal) gas constant, J/(mol·K)
	T R;
	// Molar mass of the atmospheric air, kg/mol
	T M;

	/*
	*	Returns the density given pressure and temperature
	*
	*	@param p	Pressure, kPa 
	*	@param t	Temperature, Kelvin
	*/
	T density(const T &p, const T &t) const {
		return p * M / (R * t);
	}

	/*
	*	Returns the pressure given temperature
	*
	*	@param t	Temperature, Kelvin
	*/
	T pressure(const T &t) const {
		float e = g*M / (R*L);
		return ro0 * glm::pow(t / T0, e);
	}

	/*
	 *	Returns the temperature at height h
	 *	
	 *	@param h	Height in meters
	 */
	T temperature_at_altitude(const T &h) const {
		return T0 - L*h;
	}

	/*
	*	Returns the pressure at height h
	*
	*	@param h	Height in meters
	*/
	T pressure_at_altitude(const T &h) const {
		float t = temperature_at_altitude(h);
		return pressure(t);
	}

	/*
	*	Returns the density at height h
	*
	*	@param h	Height in meters
	*/
	T density_at_altitude(const T &h) const {
		float t = temperature_at_altitude(h);
		float p = pressure(t);
		return density(p, t);
	}
};

auto inline atmospherics_earth_properties() {
	atmospherics_properties<float> ap;

	ap.ro0 = 101.325f;	// kPa
	ap.T0 = 288.15f;	// K
	ap.g = 9.80665f;	// m/s^2
	ap.L = 0.0065f;		// K/m
	ap.R = 8.31447f;	// J/(mol·K)
	ap.M = 0.0289644f;	// kg/mol

	ap.phase = -.88f;
	ap.rayleigh_scattering_coefficient = glm::vec3{ 5.8f, 13.5f, 33.1f } * 1e-6f;
	ap.mie_scattering_coefficient = 2e-5f;
	ap.mie_absorption_coefficient = 2.2222e-6f;

	return ap;
}

}
}
