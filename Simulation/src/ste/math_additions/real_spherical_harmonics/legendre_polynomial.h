// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <functional>

namespace StE {

namespace _legendre_polynomial_utils {
template <unsigned x>
struct double_factorial {
	static constexpr unsigned val = x * double_factorial<x - 2>::val;
};
template <>
struct double_factorial<1> {
	static constexpr unsigned val = 1;
};
template <>
struct double_factorial<0> {
	static constexpr unsigned val = 1;
};
}

template <unsigned m, unsigned l, typename = void>
class legendre_polynomial {};

template <unsigned m, unsigned l>
class legendre_polynomial<m, l, typename std::enable_if_t<l != m + 1 && m != l>> {
	static_assert(m <= l, "Band index m must not be greater than l");
public:
	float operator()(const float &x) const {
		float d = 1.f / (l - m);
		return d * (x * (2 * l - 1) * legendre_polynomial<m, l - 1>()(x) - (l + m - 1) * legendre_polynomial<m, l - 2>()(x));
	}
};

template <unsigned m, unsigned l>
class legendre_polynomial<m, l, typename std::enable_if_t<l == m + 1>> {
public:
	float operator()(const float &x) const {
		return x * (2 * m + 1) * legendre_polynomial<m, m>()(x);
	}
};

template <unsigned m>
class legendre_polynomial<m, m> {
public:
	float operator()(const float &x) const {
		auto df = _legendre_polynomial_utils::double_factorial<2 * m - 1>::val;
		auto one = glm::pow(-1, m);
		return one * df * glm::pow(1 - x*x, static_cast<float>(m) / 2.f);
	}
};

template <>
class legendre_polynomial<0, 0> {
public:
	float operator()(const float &x) const {
		return 1.f;
	}
};

}
