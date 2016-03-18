// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "legendre_polynomial.hpp"

#include <functional>

namespace StE {

namespace _rsh_utils {

template <unsigned x>
struct factorial {
	static constexpr unsigned val = x * factorial<x - 1>::val;
};
template <>
struct factorial<0> {
	static constexpr unsigned val = 1;
};

template <unsigned m, unsigned l>
struct K {
	static constexpr float _x = (2 * l + 1) * 0.159154943091895335768883763372514362f / 2.f;
	static constexpr unsigned _f1 = factorial<l - m>::val;
	static constexpr unsigned _f2 = factorial<l + m>::val;
	float operator()() const {
		return glm::sqrt(_x * static_cast<float>(_f1) / static_cast<float>(_f2));
	}
};

}

template <int m, unsigned l, typename = void>
class RSH {};

template <int m, unsigned l>
class RSH<m, l, typename std::enable_if_t<0 < m>> {
public:
	float operator()(float theta, float phi) const {
		return glm::root_two<float>() * _rsh_utils::K<m, l>()() * glm::cos(m * phi) * legendre_polynomial<m, l>()(glm::cos(theta));
	}
};

template <int m, unsigned l>
class RSH<m, l, typename std::enable_if_t<m < 0>> {
public:
	float operator()(float theta, float phi) const {
		return glm::root_two<float>() * _rsh_utils::K<-m, l>()() * glm::sin(-m * phi) * legendre_polynomial<-m, l>()(glm::cos(theta));
	}
};

template <unsigned l>
class RSH<0, l> {
public:
	float operator()(float theta, float phi) const {
		return _rsh_utils::K<0, l>()() * legendre_polynomial<0, l>()(glm::cos(theta));
	}
};

}
