// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <array>

#include "function_traits.hpp"

namespace StE {

/**
*	@brief	Implementation of Romberg's method for numerical integration
*/
template <unsigned N>
class romberg_integration {
	static_assert(N > 0, "Iterations count must be positive.");

private:
	// Calculates the partial sum of trapezoids using half of n equally spaced segments between a and b.
	template <typename F>
	static typename function_traits<F>::result_t trapezoid(const F &f, double h, double a, double b, std::uint64_t n) {
		using T = typename function_traits<F>::result_t;

		T t = T(0);

		for (std::uint64_t i=1; i<n; i+=2) {
			double x = a + static_cast<double>(i) * h;
			t += f(x);
		}

		return t;
	}

public:
	/**
	*	@brief	Evaluate definite integral
	*
	*	https://en.wikipedia.org/wiki/Romberg's_method
	*
	* 	@param f	Function to integrate
	* 	@param a	Interval start
	* 	@param b	Interval end
	*/
	template <typename F>
	static typename function_traits<F>::result_t integrate(const F &f, double a, double b) {
		using T = typename function_traits<F>::result_t;

		if (a >= b)
			return T(0);

		T t;
		std::array<T, N> Tk;
		Tk.fill(T(0));

		Tk[0] = .5 * (b - a) * (f(a) + f(b));

		for (std::uint64_t n = 1; n < N; ++n) {
			std::uint64_t k = static_cast<std::uint64_t>(1) << n;
			double h = (b - a) / static_cast<double>(k);
			t = .5 * Tk[0] + h * trapezoid(f, h, a, b, k);

			for (std::uint64_t m = 1; m <= n; ++m) {
				T s = t + (t - Tk[m - 1]) / static_cast<double>((1 << 2 * m) - 1);
				Tk[m - 1] = t;
				t = s;
			}

			Tk[n] = t;
		}

		return Tk[N - 1];
	}
};

};
