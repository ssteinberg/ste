// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.h"

namespace StE {

struct newton_raphson_iterative_root_finding {

	/**
	*	@brief	Find a real function's root.
	*	Returns true on success, false otherwise.
	*
	*	https://en.wikipedia.org/wiki/Newton%27s_method
	*
	* 	@param f		Function to evaluate
	* 	@param d		df/dx
	* 	@param x0		Initial root guess. Root approximation output is written here.
	* 	@param epsilon	Acceptable approximation error constant
	*/
	template <int max_iterations = 1000, typename F1, typename F2>
	static bool find_roots(const F1 &f, const F2 &d, double &x0, double epsilon = 1e-10) {
		double fx;
		for (int i=0; i<max_iterations; ++i) {
			fx = f(x0);
			if (glm::abs(fx) < epsilon)
				return true;

			x0 = x0 - fx / d(x0);
		}

		return false;
	}

};

}
