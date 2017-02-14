// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <functional>
#include <utility>
#include <tuple>
#include <iostream>

namespace StE {

template<class Ch, class Tr, class Tuple, std::size_t... Is>
void print_tuple_impl(std::basic_ostream<Ch,Tr>& os,
					  const Tuple & t,
					  std::index_sequence<Is...>) {
	using swallow = int[]; // guaranties left to right order
	(void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
}

template<class Ch, class Tr, class... Args>
decltype(auto) print_tuple(std::basic_ostream<Ch, Tr>& os,
						   const std::tuple<Args...>& t) {
	os << "<";
	print_tuple_impl(os, t, std::index_sequence_for<Args...>{});
	return os << ">";
}

}
