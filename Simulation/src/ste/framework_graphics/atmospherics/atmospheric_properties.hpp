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
struct atmospheric_properties {
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

}
}
