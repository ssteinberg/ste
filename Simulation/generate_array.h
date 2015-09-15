// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <array>

namespace StE {

template<typename T, T... args> struct _generate_array_holder { static constexpr std::array<T, sizeof...(args)> data = { args... }; };

template<std::size_t N, template<int> class Populator, decltype(Populator<0>::value)... args>
struct _generate_array_impl {
	using result = _generate_array_impl<N - 1, Populator, Populator<N>::value, args...>::result;
};
template<template<int> class Populator, decltype(Populator<0>::value)... args>
struct _generate_array_impl<0, Populator, args...> {
	using result = _generate_array_holder<decltype(Populator<0>::value), Populator<0>::value, args...>;
};

template<std::size_t N, template<int> class Populator>
struct generate_array {
	using result = _generate_array_impl<N - 1, Populator>::result;
};

}
