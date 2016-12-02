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

	// Phase coefficient for the Mie scattering phase function
	T phase;

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

	return ap;
}

}
}
