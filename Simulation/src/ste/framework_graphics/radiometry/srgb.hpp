//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

/**
 *	@brief	Converts a color component value from non-linear sRGB colorspace to linear space. 
 *			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
 */
template <typename T>
T sRGB_to_linear(const T &srgb) {
	static constexpr double sRGB_to_linear_cutoff = 0.0404482362771082;

	if (srgb <= static_cast<T>(0))
		return static_cast<T>(0);
	if (srgb <= static_cast<T>(sRGB_to_linear_cutoff))
		return srgb / static_cast<T>(12.92);
	return glm::pow<T>((srgb + static_cast<T>(0.055)) / static_cast<T>(1.055), static_cast<T>(2.4));
}

/**
*	@brief	Converts a color component value from linear space to the corresponding electrical signal (sRGB colorspace).
*			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
*/
template <typename T>
T linear_to_sRGB(const T &linear) {
	static constexpr double linear_to_sRGB_cutoff = 0.00313066844250063;

	if (linear <= static_cast<T>(0))
		return static_cast<T>(0);
	if (linear <= static_cast<T>(linear_to_sRGB_cutoff))
		return linear * static_cast<T>(12.92);
	return static_cast<T>(1.055) * glm::pow<T>(linear, static_cast<T>(1. / 2.4)) - static_cast<T>(0.055);
}

}
}
