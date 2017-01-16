// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.h"
#include "newton_raphson_iterative_root_finding.hpp"

#include <array>

namespace StE {

template <typename T, int n>
struct legendre_polynomial {
	T operator()(const T &x) const {
		std::array<T, n + 1> d;
		d[0] = legendre_polynomial<T, 0>()(x);
		d[1] = legendre_polynomial<T, 1>()(x);
		for (int i = 2; i <= n; ++i)
			d[i] = (static_cast<T>(2 * i - 1) * x * d[i - 1] -
					static_cast<T>(i - 1) * d[i - 2])
			/ static_cast<T>(i);
		return d[n];
	}
};

template <typename T>
struct legendre_polynomial<T, 0> {
	T operator()(const T &x) const { return static_cast<T>(1); }
};

template <typename T>
struct legendre_polynomial<T, 1> {
	T operator()(const T &x) const { return x; }
};


template <typename T, int n>
struct legendre_polynomial_derivative {
	T operator()(const T &x) const {
		std::array<T, n + 1> d;
		d[0] = legendre_polynomial<T, 0>()(x);
		d[1] = legendre_polynomial<T, 1>()(x);
		for (int i = 2; i <= n; ++i)
			d[i] = (static_cast<T>(2 * i - 1) * x * d[i - 1] -
					static_cast<T>(i - 1) * d[i - 2])
			/ static_cast<T>(i);

		return (x * d[n] - d[n - 1]) * static_cast<T>(n) / (x*x - 1);
	}
};

template <typename T>
struct legendre_polynomial_derivative<T, 0> {
	T operator()(const T &x) const { return 0; }
};

template <typename T>
struct legendre_polynomial_derivative<T, 1> {
	T operator()(const T &x) const { return 1; }
};

/**
*	@brief	Find the i-th root of an n-th order legendere polynomial.
*/
template <int n>
double legendre_polynomial_node(int i) {
	static_assert(n > 0, "No roots for P0");
	assert(n > i && "Pn has n roots, i must be less than n");

	auto t = (static_cast<double>(i + 1) - .25) / (static_cast<double>(n) + .5);
	auto x0 = glm::cos(glm::pi<double>() * t);
	bool res = newton_raphson_iterative_root_finding::find_roots<1000000>([](double x) { return legendre_polynomial<double, n>()(x); },
																		  [](double x) { return legendre_polynomial_derivative<double, n>()(x); },
																		  x0,
																		  1e-12);

	assert(res && "Root not found");

	return x0;
}

/**
*	@brief	Find the weight of a node of an n-th order legendere polynomial.
*/
template <int n>
double legendre_polynomial_weight(double xi) {
	static_assert(n > 0, "No roots for P0");

	auto t = legendre_polynomial_derivative<double, n>()(xi);
	return 2. / ((1. - xi*xi) * t*t);
}

}
