//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <functional>

namespace StE {

template <typename T>
struct ste_resource_creator {
	template <typename ... Params>
	using is_constructible_with = std::is_constructible<T, Params...>;

	template <typename... Ts>
	T operator()(Ts&&... params) { return T(std::forward<Ts>(params)...); }
};

}
