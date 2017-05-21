// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <limits>

namespace ste {

/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Attenuation coefficients per channel.  [0, inf]
*	@param path_length	Path length.  [0, inf]
*/
template <typename T>
T beer_lambert(const T &att, const T &path_length) {
	return glm::exp(-glm::max(std::numeric_limits<T>::min(), path_length) * att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Attenuation coefficients per channel.  [0, inf]
*	@param path_length	Path length.  [0, inf]
*/
template <typename T>
glm::tvec2<T> beer_lambert(const glm::tvec2<T> &att, const T &path_length) {
	return glm::exp(-glm::max(std::numeric_limits<T>::min(), path_length) * att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Attenuation coefficients per channel.  [0, inf]
*	@param path_length	Path length.  [0, inf]
*/
template <typename T>
glm::tvec3<T> beer_lambert(const glm::tvec3<T> &att, const T &path_length) {
	return glm::exp(-glm::max(std::numeric_limits<T>::min(), path_length) * att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Attenuation coefficients per channel.  [0, inf]
*	@param path_length	Path length.  [0, inf]
*/
template <typename T>
glm::tvec4<T> beer_lambert(const glm::tvec4<T> &att, const T &path_length) {
	return glm::exp(-glm::max(std::numeric_limits<T>::min(), path_length) * att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Total attenuation per channel.  [0, inf]
*/
template <typename T>
T beer_lambert(const T &att) {
	return glm::exp(-att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Total attenuation per channel.  [0, inf]
*/
template <typename T>
glm::tvec2<T> beer_lambert(const glm::tvec2<T> &att) {
	return glm::exp(-att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Total attenuation per channel.  [0, inf]
*/
template <typename T>
glm::tvec3<T> beer_lambert(const glm::tvec3<T> &att) {
	return glm::exp(-att);
}
/*
*	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
*
*	@param att			Total attenuation per channel.  [0, inf]
*/
template <typename T>
glm::tvec4<T> beer_lambert(const glm::tvec4<T> &att) {
	return glm::exp(-att);
}

}
