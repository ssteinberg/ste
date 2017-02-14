// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <newton_raphson_iterative_root_finding.hpp>

namespace StE {

template <typename T, int n>
struct legendre_polynomial {
	static_assert(n >= 2, "n must be non-negative");

	T operator()(const T &x) const {
		T t[2];
		t[0] = legendre_polynomial<T, 0>()(x);
		t[1] = legendre_polynomial<T, 1>()(x);

		int idx;
		for (int i = 2; i <= n; ++i) {
			idx = i % 2;
			t[idx] = (static_cast<T>(2 * i - 1) * x * t[idx ^ 0x1] -
					  static_cast<T>(i - 1) * t[idx]) / static_cast<T>(i);
		}
		return t[idx];
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
	static_assert(n >= 2, "n must be non-negative");

	T operator()(const T &x) const {
		T t[2];
		t[0] = legendre_polynomial<T, 0>()(x);
		t[1] = legendre_polynomial<T, 1>()(x);

		int idx;
		for (int i = 2; i <= n; ++i) {
			idx = i % 2;
			t[idx] = (static_cast<T>(2 * i - 1) * x * t[idx ^ 0x1] -
					  static_cast<T>(i - 1) * t[idx]) / static_cast<T>(i);
		}

		auto Pn = t[idx];
		auto Pn_1 = t[idx ^ 0x1];

		return (x * Pn - Pn_1) * static_cast<T>(n) / (x*x - 1);
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
*	@brief	Find the i-th root (node) of an n-th order legendere polynomial.
*/
template <int n>
double legendre_polynomial_node(int i) {
	static_assert(n > 0, "No roots for P0");
	assert(i >= 0 && "i must be non-negative");
	assert(n > i && "Pn has n roots, i must be less than n");

	// Initial guess
	auto t = (static_cast<double>(i + 1) - .25) / (static_cast<double>(n) + .5);
	auto x0 = glm::cos(glm::pi<double>() * t);

	bool res = newton_raphson_iterative_root_finding::find_root<1000000>([](double x) { return legendre_polynomial<double, n>()(x); },
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
