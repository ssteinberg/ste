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
		constexpr T max_density = 1e-5;

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
			return len * pressure(glm::mix(h0, h1, static_cast<T>(.5)), H);
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
	*	Solves the multiple scattering equation in participating medium up to order k by numerical integration.
	*	When k == 1, evaluates the single scattering term.
	*	Returns the intensity of light that reaches the observer given a directional light.
	*
	*	@param N	Template parameter that controls numerical integration steps for a single scatter accumulation
	*	@param M	Template parameter that controls numerical integration steps for a spherical gather accumulation step
	*	@param k	Specifies multiple-scattering max order (>=1), with 1 being single scatter
	*	@param P0	Observer point in world coordinates
	*	@param V	Normalized view direction
	*	@param L	Normalized direction of directional light source
	*	@param I0	Spectral intensity of light source
	*/
	template <int N, int M>
	Vec scatter(int k, const Vec &P0, const Vec &V, const Vec &L, const RGB &I0) const {
		static_assert(N >= 1, "Expected positive N");

		auto Hr = scale_height();
		auto Hm = scale_height_aerosols();
		auto Hmax_r = max_height(Hr);
		auto Hmax_m = max_height(Hm);

		Vec accumulated = Vec(static_cast<T>(0));
		for (int i=1; i<=k; ++i) {
			auto r = scatter<N, M>(i, P0, V, L, I0,
								   this->rayleigh_scattering_coefficient, this->rayleigh_extinction_coeffcient(),
								   Hr, Hmax_r,
								   [](const T &c) { return rayleigh_phase_function(c); });
			auto m = scatter<N, M>(i, P0, V, L, I0,
								   Vec(this->mie_scattering_coefficient), Vec(this->mie_extinction_coeffcient()),
								   Hm, Hmax_m,
								   [this](const T &c) { return cornette_shanks_phase_function(c, this->phase); });

			accumulated += r + m;
		}

		return accumulated;
	}

private:
	// Scattering helper functions
	template <int N, int  M>
	Vec scatter(int k, const Vec &P0, const Vec &V, const Vec &L, const RGB &I0,
				const Vec &scatter_coefficient, const Vec &extinction_coefficient,
				const T &H, const T &Hmax,
				const std::function<T(const T &)> &Fphase) const {
		//! Currently the planet is flat...
		Vec up ={ 0, 1, 0 };
		auto h0 = glm::max(P0.y, static_cast<T>(0));

		auto delta = glm::dot(up, V);
		if (delta == 0 || h0 >= Hmax)
			return Vec(static_cast<T>(0));

		auto h1 = delta > 0 ? Hmax : 0;

		auto path_length = (h1 - h0) / delta;
		auto l = path_length / static_cast<T>(N);

		auto cos_theta = glm::dot(V, L);
		auto F = Fphase(cos_theta);

		Vec I0v = Vec{ static_cast<T>(I0.R()), static_cast<T>(I0.G()), static_cast<T>(I0.B()) };

		Vec gather = Vec(static_cast<T>(0));
		for (int i=0; i<N; ++i) {
			Vec P = P0 + (static_cast<double>(i) + .5) * l * V;
			auto h = P.y;
			auto scatter = scatter_coefficient * pressure(h, H);

			auto t = extinction_coefficient * optical_length(P0, P, H);
			if (k == 1) {
				// For first order scatter we need to take into account the attenuation from
				auto t2 = extinction_coefficient * optical_length_from_infinity(P, L, H);
				t += t2;
			}

			// For first order scatter we sample the actual amount of scattered light at point P from the light source.
			// For multiple scattering we sample the gathered scatters across a 4*pi steradians at point P.
			Vec sample;
			if (k == 1) {
				sample = I0v * F * scatter * beer_lambert(t);
			}
			else {
				auto G = scatter_gather<N, M>(k-1, P, V, L, I0,
											  scatter_coefficient, extinction_coefficient,
											  H, Hmax, Fphase);
				sample = G * scatter * beer_lambert(t);
			}

			gather += l * sample;
		}

		return gather;
	}
	template <int N, int M>
	Vec scatter_gather(int k, const Vec &P0, const Vec &V, const Vec &L, const RGB &I0,
					   const Vec &scatter_coefficient, const Vec &extinction_coefficient,
					   const T &H, const T &Hmax,
					   const std::function<T(const T &)> &Fphase) const {
		static_assert(M > 1, "M should be positive");
		static_assert(M % 2 == 1, "M should be odd");

		Vec result = Vec(static_cast<T>(0));

		/*
		 *	Numerically integrate over a sphere. The phase function is already normalized, i.e.
		 *	integral(Fphase) across 4*pi == 1
		 *	Therefore only need to normalize by division by the total samples weight, which is given by the phase function.
		 */

		T ft = 0;
		for (int i=0; i<M; ++i) {
			T theta = static_cast<T>(i) / static_cast<T>(M - 1) * glm::pi<T>();
			T sin_theta = glm::sin(theta);

			int longtitude_samples = i == 0 || i == M-1 ?
				1 :
				glm::max(3, static_cast<int>(glm::ceil(sin_theta * static_cast<T>(M << 1))));
			for (int j=0; j<longtitude_samples; ++j) {
				T phi = static_cast<T>(j << 1) / static_cast<T>(longtitude_samples) * glm::pi<T>();
				Vec omega ={ sin_theta * glm::cos(phi), sin_theta * glm::sin(phi), glm::cos(theta) };

				auto sample = scatter<N, M>(k, P0, omega, L, I0,
											scatter_coefficient, extinction_coefficient,
											H, Hmax, Fphase);
				auto F = Fphase(glm::dot(-omega, V));

				result += F * sample;
				ft += F;
			}
		}

		return result / ft;
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
	ap.M_aerosols = 0.223;	// kg/mol

	ap.phase = -.88;
	ap.rayleigh_scattering_coefficient = glm::dvec3{ 5.7, 13.36, 32.77 } * 1e-8;
	ap.mie_scattering_coefficient = 2e-7;
	ap.mie_absorption_coefficient = 2.2e-8;

	return ap;
}

}
}
