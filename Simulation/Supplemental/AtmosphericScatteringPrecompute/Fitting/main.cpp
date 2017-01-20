// Fitting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <future>

#include "scatter.hpp"
#include "sampling.hpp"


int main() {
//	static constexpr int S = 1000;
//	static constexpr int N = 9;
//	static constexpr int M = 90;
//
//	auto lambda = [&](double sample_theta, double sample_phi, const glm::dvec3 &V) {
//		double sin_theta = glm::sin(sample_theta);
//		glm::tvec3<double> omega = { sin_theta * glm::cos(sample_phi), sin_theta * glm::sin(sample_phi), glm::cos(sample_theta) };
//		return cornette_shanks_phase_function(-glm::dot(omega, V), -.95);
//	};
//
//	auto qp = StE::gaussian_quadrature_spherical_integration<M>::generate_points();
//
//	std::array<double, S> results;
//	for (int i = 0; i < S; ++i) {
//		auto sp = StE::sample_sphere_surface_uniform<3, double>();
//		auto V = glm::dvec3{sp[0],sp[1],sp[2]};
//		results[i] = StE::gaussian_quadrature_spherical_integration<M>::integrate([&](double x, double y) {
//			return lambda(x, y, V);
//		}, qp);
//	}
//	double everage = 0;
//	double max = .0;
//	for (auto &r : results) {
//		max = glm::max(max, glm::abs(r - 1.));
//		everage += 1. / static_cast<double>(S) * r;
//	}
//
//	std::cout << everage << std::endl;
//	std::cout << max << std::endl;

	auto atmosphere = StE::Graphics::atmospherics_earth_properties({ 0,-6.371e+6,0 });

	StE::Graphics::atmospherics_precompute_scattering aps(atmosphere);

//	aps.load("atmospherics_scatter_lut.bin");

	aps.build_optical_length_lut();
	aps.build_scatter_lut(7);
	aps.build_ambient_lut();

	aps.write_out("atmospherics_scatter_lut.bin");

	return 0;
}
