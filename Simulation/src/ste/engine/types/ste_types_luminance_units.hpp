//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <numerical_type.hpp>
#include <ste_types_length_unit.hpp>

namespace ste {

namespace _detail {

struct cd_unique_tag {};
struct nit_unique_tag {};

}

/*
*	@brief	The candela (cd) is the SI base unit of luminous intensity; that is, luminous power per unit solid angle emitted by a point light source in a particular direction.
*			Luminous intensity is analogous to radiant intensity, but instead of simply adding up the contributions of every wavelength of light in the source's spectrum, 
*			the contribution of each wavelength is weighted by the standard luminosity function (a model of the sensitivity of the human eye to different wavelengths).
*/
using cd_t = numerical_type<float, _detail::cd_unique_tag>;

inline auto operator"" _cd(unsigned long long int val) { return cd_t(static_cast<cd_t::value_type>(val)); }
inline auto operator"" _cd(long double val) { return cd_t(static_cast<cd_t::value_type>(val)); }

/*
*	@brief	The candela per square metre (cd/m2) is the derived SI unit of luminance. The unit is based on the candela, the SI unit of luminous intensity, and the square metre, the SI unit of area.
*			Nit (nt) is a non-SI name also used for this unit 
*/
using nit_t = numerical_type<float, _detail::nit_unique_tag>;

inline auto operator"" _nt(unsigned long long int val) { return nit_t(static_cast<nit_t::value_type>(val)); }
inline auto operator"" _nt(long double val) { return nit_t(static_cast<nit_t::value_type>(val)); }
inline auto operator"" _cd_m²(unsigned long long int val) { return nit_t(static_cast<nit_t::value_type>(val)); }
inline auto operator"" _cd_m²(long double val) { return nit_t(static_cast<nit_t::value_type>(val)); }

/*
*	@brief	Division of candela by area gives nits.
*/
template <int Exp>
constexpr auto operator/(cd_t cd,
						 ste_length_type<Exp, 2> area) noexcept {
	using area_m2_t = square_metre;

	const auto t = area_m2_t(area);
	const auto nits = nit_t(static_cast<cd_t::value_type>(cd) / static_cast<area_m2_t::value_type>(t));

	return nits;
}

}
