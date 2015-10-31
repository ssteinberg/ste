// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "surface_traits.h"

#include <functional>
#include <type_traits>
#include <limits>

namespace StE {
namespace LLR {

template <gli::format T>
float surface_element_to_sfloat(const surface_element_type<T>::type &v, std::enable_if_t<!surface_is_normalized<T>::value>* = 0) {
	return static_cast<float>(v);
}

template <gli::format T>
float surface_element_to_sfloat(const surface_element_type<T>::type &v, std::enable_if_t<surface_is_normalized<T>::value>* = 0) {
	float f = static_cast<float>(v);
	return f / static_cast<float>(std::numeric_limits<surface_element_type<T>::type>::max());
}

template <gli::format T>
surface_element_type<T>::type surface_element_from_sfloat(float f, std::enable_if_t<!surface_is_normalized<T>::value>* = 0) {
	return static_cast<surface_element_type<T>::type>(f);
}

template <gli::format T>
surface_element_type<T>::type surface_element_from_sfloat(float f, std::enable_if_t<surface_is_normalized<T>::value>* = 0) {
	float denorm = f * static_cast<float>(std::numeric_limits<surface_element_type<T>::type>::max());
	return static_cast<surface_element_type<T>::type>(denorm);
}

template <gli::format From, gli::format To>
surface_element_type<To>::type surface_element_cast(const surface_element_type<From>::type &v, std::enable_if_t<!surface_is_normalized<From>::value && !surface_is_normalized<To>::value>* = 0) {
	return static_cast<surface_element_type<To>::type>(v);
}

template <gli::format From, gli::format To>
surface_element_type<To>::type surface_element_cast(const surface_element_type<From>::type &v, std::enable_if_t<surface_is_normalized<From>::value || surface_is_normalized<To>::value>* = 0) {
	float inter = surface_element_to_sfloat<From>(v);
	return surface_element_from_sfloat<To>(inter);
}

}
}
