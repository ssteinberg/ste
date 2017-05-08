// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <array>
#include <random>

namespace ste {

/*
*	Sample uniform points on the surface of a k-dimensional sphere.
*
*	@param k				Dimensions
*	@param Gen, generator	Random number generator
*/
template <int k, typename T, typename Gen>
std::array<T, k> sample_sphere_surface_uniform(Gen &generator) {
	std::normal_distribution<T> d(0, 1);

	std::array<T, k> result;
	T mag = static_cast<T>(0);
	for (int i=0; i<k; ++i) {
		auto t = d(generator);
		result[i] = t;
		mag += t*t;
	}

	mag = glm::sqrt(mag);
	for (T &t : result)
		t /= mag;

	return result;
}

/*
*	Sample uniform points on the surface of a 2-dimensional sphere.
*
*	@param k	Dimensions
*/
template <int k, typename T>
std::array<T, k> sample_sphere_surface_uniform() {
	std::random_device rd;
	std::mt19937 gen(rd());

	return sample_sphere_surface_uniform<k, T, decltype(gen)>(gen);
}

}
