// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <array>

#include "function_traits.hpp"

namespace StE {

/**
*	@brief	Implementation of Romberg's method for numerical integration
*/
template <int N>
class romberg_integration {
private:
	// Calculates the sum of n-1 trapezoids using n equally spaced segments between a and b.
	// n >= 1
	template <typename F>
	static float trapezoid(const F &f, float a, float b, int n) {
		assert(n >= 1);

		float c = (b - a) / static_cast<float>(n);
		float t = .0f;

		for (int i=1; i<n; ++i) {
			float x = a + static_cast<float>(i) / static_cast<float>(n) * (b - a);
			t += c * static_cast<float>(f(x));
		}

		t += .5f * c * static_cast<float>(f(a) + f(b));

		return t;
	}

public:
	/**
	*	@brief	Evaluate definite integral
	*	
	*	https://en.wikipedia.org/wiki/Romberg's_method
	*
	* 	@param f	funtion to integrate
	* 	@param a	Interval start
	* 	@param b	Interval end
	*/
	template <typename F>
	static typename function_traits<F>::result_t integrate(const F &f, float a, float b) {
		using T = typename function_traits<F>::result_t;

		float t = .0f;
		std::array<float, N> Tk;
		Tk.fill(.0f);

		for (int k = 1; k <= N; ++k) {
			int segments = 1 << (k - 1);
			t = trapezoid(f, a, b, segments);

			for (int i = 1; i < k; ++i) {
				float four_i = 2 << i;

				float s = (four_i * t - Tk[i - 1]) / (four_i - 1.f);
				Tk[i - 1] = t;
				t = s;
			}
		}

		return t;
	}
};

};
