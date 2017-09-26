//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <tuple>
#include <utility>
#include <type_traits>

namespace ste {

namespace _detail {

template <typename T, typename Seq>
struct tuple_reverse_impl;
template <typename T, std::size_t... I>
struct tuple_reverse_impl<T, std::index_sequence<I...>> {
	using type = std::tuple<typename std::tuple_element<sizeof...(I) - 1 - I, T>::type...>;
};
template <typename T>
struct tuple_reverse_impl<T, std::index_sequence<>> {
	using type = T;
};

template <typename T, std::size_t... indices>
auto invert_tuple(T &&tuple, std::index_sequence<indices...>) {
	using tuple_t = std::decay_t<T>;
	constexpr auto tuple_size = std::tuple_size_v<tuple_t>;

	return std::tuple<std::tuple_element_t<tuple_size - indices - 1, tuple_t>...>(std::get<tuple_size - indices - 1>(std::forward<T>(tuple))...);
}

}

template <typename T>
struct tuple_reverse : _detail::tuple_reverse_impl<T, std::make_index_sequence<std::tuple_size<T>::value>> {};
template <typename T>
using tuple_reverse_t = typename tuple_reverse<T>::type;

template <typename T>
auto invert_tuple(T &&tuple) {
	return _detail::invert_tuple(std::forward<T>(tuple),
								 std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>());
}

}
