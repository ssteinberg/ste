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

template <int m, unsigned l, typename = void>
class RSH {};

template <int m, unsigned l>
class RSH<m, l, typename std::enable_if_t<0 < m>> {
public:
	float operator()(float theta, float phi) const {
		return glm::root_two<float>() * _rsh_utils::K<m, l>()() * glm::cos(m * phi) * 
			_rsh_utils::legendre_polynomial<m, l>()(glm::cos(theta));
	}
};

template <int m, unsigned l>
class RSH<m, l, typename std::enable_if_t<m < 0>> {
public:
	float operator()(float theta, float phi) const {
		return glm::root_two<float>() * _rsh_utils::K<-m, l>()() * glm::sin(-m * phi) * 
			_rsh_utils::legendre_polynomial<-m, l>()(glm::cos(theta));
	}
};

template <unsigned l>
class RSH<0, l> {
public:
	float operator()(float theta, float phi) const {
		return _rsh_utils::K<0, l>()() * 
			_rsh_utils::legendre_polynomial<0, l>()(glm::cos(theta));
	}
};

}
