// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <tuple>

namespace ste {

namespace _detail {

template <int N, int Size, typename Tuple>
struct inherit_from_tuple_types_impl : std::tuple_element_t<N, Tuple>, inherit_from_tuple_types_impl<N + 1, Size, Tuple> {};
template <int Size, typename Tuple>
struct inherit_from_tuple_types_impl<Size, Size, Tuple> {};

}

template <typename T, typename... Ts>
struct inherit_from_types : T, inherit_from_types<Ts...> {};
template <typename T>
struct inherit_from_types<T> : T {};

template <typename Tuple>
struct inherit_from_tuple_types : _detail::inherit_from_tuple_types_impl<0, std::tuple_size_v<Tuple>, Tuple> {};

}
