// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "beer_lambert.hpp"
#include "rayleigh_scattering.hpp"
#include "mie_scattering.hpp"

#include "RGB.hpp"

#include "optional.hpp"

#include <limits>
#include <map>

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
		//! Currently the planet is flat...
		Vec up ={ 0, 1, 0 };
		T h1 = glm::max(P1.y, static_cast<T>(0));

		T delta = glm::dot(up, -V);
		// Ignore rays coming from below the ground
		if (delta <= 0)
			return +std::numeric_limits<T>::infinity();

		T normalizer = static_cast<T>(1) / delta;

		return normalizer * ro0 * H * glm::exp(-h1 / H);
	}

	/*
	*	Solves the single scattering equation by numerical integration.
	*	Returns the intensity of light that reaches the observer given a directional light.
	*
	*	@param N	Template parameter that controls numerical integration steps
	*	@param P0	Observer point in world coordinates
	*	@param V	Normalized view direction
	*	@param L	Normalized direction of directional light source
	*	@param I0	Spectral intensity of light source
	*/
	template <int N>
	Vec single_scatter(const Vec &P0, const Vec &V, const Vec &L, const RGB &I0) const {
		constexpr T max_density = 1e-5;

		auto Hr = scale_height();
		auto Hm = scale_height_aerosols();
		auto Hr_max = -Hr * glm::log(max_density);
		auto Hm_max = -Hm * glm::log(max_density);
		auto Hmax = glm::max(Hr_max, Hm_max);

		//! Currently the planet is flat...
		Vec up = { 0, 1, 0 };
		auto h0 = glm::max(P0.y, static_cast<T>(0));

		auto delta = dot(up, V);
		if (delta == 0 || h0 >= Hmax)
			return Vec(static_cast<T>(0));

		auto h1 = delta > 0 ? Hmax : 0;

		auto path_length = (h1 - h0) / delta;
		auto l = path_length / static_cast<T>(N);

		auto cos_theta = glm::dot(V, L);
		auto Fr = rayleigh_phase_function(cos_theta);
		auto Fm = cornette_shanks_phase_function(cos_theta, this->phase);

		Vec gather = Vec(static_cast<T>(0));
		for (int i=0; i<N; ++i) {
			Vec P = P0 + (static_cast<double>(i) + .5) * l * V;
			auto h = P.y;
			
			auto t_r = this->rayleigh_extinction_coeffcient() * (this->optical_length(P0, P, Hr) + 
																 this->optical_length_from_infinity(P, L, Hr));
			auto t_m = this->mie_extinction_coeffcient() * (this->optical_length(P0, P, Hm) + 
															this->optical_length_from_infinity(P, L, Hm));

			auto scatter_r = this->rayleigh_scattering_coefficient * this->pressure(h);
			auto scatter_m = this->mie_scattering_coefficient * this->pressure_aerosols(h);

			auto r = Fr * scatter_r * beer_lambert(t_r);
			auto m = Fm * scatter_m * beer_lambert(t_m);

			gather += l * (Vec(m) + r);
		}

		glm::vec3 I0v = I0;
		return Vec{ static_cast<T>(I0v.x), static_cast<T>(I0v.y), static_cast<T>(I0v.z) } * gather / (4 * glm::pi<T>());
	}

	/*
	*	Solves the multiple scattering equation in participating medium up to order k by numerical integration.
	*	Returns the intensity of light that reaches the observer given a directional light.
	*
	*	@param k	Template parameter, specifies multiple-scattering max order (>=1), with 1 being single scatter
	*	@param N	Template parameter that controls numerical integration steps for a single scatter accumulation
	*	@param M	Template parameter that controls numerical integration steps for a spherical gather accumulation step
	*	@param P0	Observer point in world coordinates
	*	@param V	Normalized view direction
	*	@param L	Normalized direction of directional light source
	*	@param I0	Spectral intensity of light source
	*/
	template <int k, int N, int M = 5>
	Vec multiple_scatter(const Vec &P0, const Vec &V, const Vec &L, const RGB &I0) const {
		static_assert(k >= 1, "k must be 1 or greater");

		if (k == 1)
			return single_scatter<N>(P0, V, L, I0);


	}

private:
	template <int k, int N, int M>
	Vec multiple_scatter_gather(const Vec &P0, const Vec &V, const Vec &L, const RGB &I0, 
								std::unordered_map<glm::ivec3, optional<T>> &cache) const {
		static_assert(k >= 1, "k must be 2 or greater");

		for (int x=0; x<2*M; ++x) {
			auto phi = static_cast<T>(x) / static_cast<T>(M) * glm::pi<T>();		// [0, 2*pi)

			for (int y=0; y<M; ++y) {
				auto theta = static_cast<T>(y) / static_cast<T>(M - 1) * glm::pi<T>();	// [0, pi]

				float sine_theta = glm::sin(theta);
				Vec V ={ sine_theta * glm::cos(phi), sine_theta * glm::sin(phi), glm::cos(theta) };
			}
		}
	}
};

auto inline atmospherics_earth_properties() {
	atmospherics_properties<double> ap;

	ap.ro0 = 101.325;		// kPa
	ap.T0 = 288.15;			// K
	ap.g = 9.80665;			// m/s^2
	ap.R = 8.31447;			// J/(mol·K)
	ap.M = 0.0289644;		// kg/mol
	ap.M_aerosols = 0.25;	// kg/mol

	ap.phase = -.88;
	ap.rayleigh_scattering_coefficient = glm::dvec3{ 5.7, 13.36, 32.77 } * 1e-8;
	ap.mie_scattering_coefficient = 2e-7;
	ap.mie_absorption_coefficient = 2.2e-8;

	return ap;
}

}
}
