// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <array>

#include "legendre_polynomial.hpp"
#include "function_traits.hpp"

namespace StE {

/**
*	@brief	Implementation of numerical spherical integration using Gaussian Quadrature.
*	See NUMERICAL INTEGRATION ON THE SPHERE, by Kendall Atkinson. J. Austral. Math. Soc. (Series B) 23 (1982), 332-347.
*/
template <unsigned N>
class gaussian_quadrature_spherical_integration {
	static_assert(N > 0, "Iterations count must be positive.");

public:
	struct quadrature_points {
		std::array<double, N> xi;
		std::array<double, N> wi;
	};

public:
	/**
	*	@brief	Evaluate definite spherical integral.
	*	f(theta, phi) is a function receiving spherical coordinates. theta : [0,pi] and phi : [0,2*pi].
	*
	* 	@param f	Function to integrate
	* 	@param p	Precomputed Gauss-Legendre nodes and weights
	*/
	template <typename F>
	static typename function_traits<F>::result_t integrate(const F &f, const quadrature_points &p) {
		using T = typename function_traits<F>::result_t;

		double t = glm::pi<double>() / static_cast<double>(N);

		T result = T(0);
		for (int j = 0; j < 2*N; ++j) {
			double phi = t * (static_cast<double>(j) + .5);
			for (int i = 0; i < N; ++i) {
				double theta = glm::acos(p.xi[i]);
				double w = p.wi[i];

				result += static_cast<T>(t * w * f(theta, phi));
			}
		}

		return result;
	}

	/**
	*	@brief	Evaluate definite spherical integral.
	*	f(theta, phi) is a function receiving spherical coordinates. theta : [0,pi] and phi : [0,2*pi].
	*
	* 	@param f	Function to integrate
	*/
	template <typename F>
	static typename function_traits<F>::result_t integrate(const F &f) {
		return integrate(f, generate_points());
	}

	static quadrature_points generate_points() {
		quadrature_points q;

		int i;
		for (i = 0; i < N / 2; ++i) {
			q.xi[i] = StE::legendre_polynomial_node<N>(i);
			q.wi[i] = StE::legendre_polynomial_weight<N>(q.xi[i]);
		}
		if (N % 2 == 1) {
			q.xi[i] = .0;
			q.wi[i] = StE::legendre_polynomial_weight<N>(q.xi[i]);

			++i;
		}
		for (; i < N; ++i) {
			q.xi[i] = -q.xi[N - 1 - i];
			q.wi[i] = q.wi[N - 1 - i];
		}

		return q;
	}
};

};
