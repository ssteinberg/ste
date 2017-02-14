// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <iostream>

namespace StE {

namespace detail {

template<typename V, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const V& v) {
	os << "<";
	for (unsigned i = 0; i < v.length(); ++i)
		os << v[i] << (i + 1 < v.length() ? ", " : "");
	return os << ">";
}

template<typename M, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const M& m) {
	auto h = m.length();
	auto w = m[0].length();

	os << "(";
	for (unsigned i = 0; i < w; ++i) {
		for (unsigned j = 0; j < h; ++j)
			os << m[j][i] << (j + 1 < h || i + 1 < w ? ", " : "");
		if (i + 1 < h)
			os << std::endl;
	}
	return os << ")";
}

}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec2<T>& v) {
	return detail::print_vec(os, v);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tvec2<T>& t) {
	return print_vec(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec3<T>& v) {
	return detail::print_vec(os, v);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tvec3<T>& t) {
	return print_vec(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_vec(std::basic_ostream<Ch, Tr>& os,
						 const glm::tvec4<T>& v) {
	return detail::print_vec(os, v);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tvec4<T>& t) {
	return print_vec(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat2x2<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat2x2<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat2x3<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat2x3<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat2x4<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat2x4<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat3x2<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat3x2<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat3x3<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat3x3<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat3x4<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat3x4<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat4x2<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat4x2<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat4x3<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat4x3<T>& t) {
	return print_mat(os, t);
}

template<class T, class Ch, class Tr>
decltype(auto) print_mat(std::basic_ostream<Ch, Tr>& os,
						 const glm::tmat4x4<T>& m) {
	return detail::print_mat(os, m);
}
template<class T>
decltype(auto) operator<<(std::ostream &os, const glm::tmat4x4<T>& t) {
	return print_mat(os, t);
}

}
