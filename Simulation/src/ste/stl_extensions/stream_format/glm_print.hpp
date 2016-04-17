// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include <iostream>

namespace StE {

namespace detail {

template<typename V, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const V& v) {
	os << "(";
	for (unsigned i = 0; i < v.length(); ++i)
		os << v[i] << (i+1 < v.length() ? ", " : "");
	return os << ")";
}

}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec2<T>& v) {
	return detail::print_vec(os, v);
}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec3<T>& v) {
	return detail::print_vec(os, v);
}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec4<T>& v) {
	return detail::print_vec(os, v);
}

}
