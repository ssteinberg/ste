// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

namespace StE {

struct error_type {};

template <typename T>
struct is_error_type {
	static constexpr bool value = false;
};
template <>
struct is_error_type<error_type> {
	static constexpr bool value = true;
};
template <typename T>
static constexpr bool is_error_type_v = is_error_type<T>::value;

}
