//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <functional>

namespace StE {

template <typename T, typename ... Params>
struct ste_resource_is_constructible {
	static constexpr bool value = std::is_constructible<T, Params...>::value;
};
template <typename T>
struct ste_resource_is_constructible<T, void> {
	static constexpr bool value = std::is_default_constructible<T>::value;
};
template <typename T>
struct ste_resource_is_constructible<T> {
	static constexpr bool value = std::is_default_constructible<T>::value;
};

template <typename T>
struct ste_resource_creator {
	template <typename... Ts>
	T operator()(Ts&&... params) { return T(std::forward<Ts>(params)...); }
};

}
