// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "surface_traits.hpp"

#include <functional>
#include <type_traits>
#include <limits>

namespace StE {
namespace Core {

template <gli::format T>
float surface_element_to_sfloat(const typename surface_element_type<T>::type &v, std::enable_if_t<!surface_is_normalized<T>::value>* = nullptr) {
	return static_cast<float>(v);
}

template <gli::format T>
float surface_element_to_sfloat(const typename surface_element_type<T>::type &v, std::enable_if_t<surface_is_normalized<T>::value>* = nullptr) {
	float f = static_cast<float>(v);
	return f / static_cast<float>(std::numeric_limits<typename surface_element_type<T>::type>::max());
}

template <gli::format T>
typename surface_element_type<T>::type surface_element_from_sfloat(float f, std::enable_if_t<!surface_is_normalized<T>::value>* = nullptr) {
	return static_cast<typename surface_element_type<T>::type>(f);
}

template <gli::format T>
typename surface_element_type<T>::type surface_element_from_sfloat(float f, std::enable_if_t<surface_is_normalized<T>::value>* = nullptr) {
	float denorm = f * static_cast<float>(std::numeric_limits<typename surface_element_type<T>::type>::max());
	return static_cast<typename surface_element_type<T>::type>(denorm);
}

template <gli::format From, gli::format To>
typename surface_element_type<To>::type surface_element_cast(const typename surface_element_type<From>::type &v, std::enable_if_t<!surface_is_normalized<From>::value && !surface_is_normalized<To>::value>* = nullptr) {
	return static_cast<typename surface_element_type<To>::type>(v);
}

template <gli::format From, gli::format To>
typename surface_element_type<To>::type surface_element_cast(const typename surface_element_type<From>::type &v, std::enable_if_t<surface_is_normalized<From>::value || surface_is_normalized<To>::value>* = nullptr) {
	float inter = surface_element_to_sfloat<From>(v);
	return surface_element_from_sfloat<To>(inter);
}

}
}
