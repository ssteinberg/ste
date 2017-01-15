// Fitting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <future>

#include "scatter.hpp"


int main() {
	auto atmosphere = StE::Graphics::atmospherics_earth_properties({ 0,-6.371e+6,0 });

	StE::Graphics::atmospherics_precompute_scattering aps(atmosphere);

	//aps.load("atmospheric_scatter_lut.bin");

	aps.build_optical_length_lut();
	aps.build_scatter_lut(10);

	aps.write_out("atmospheric_scatter_lut.bin");

	return 0;
}
