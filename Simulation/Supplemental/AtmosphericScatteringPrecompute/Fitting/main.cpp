// Fitting.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "scatter.hpp"


int main() {
	auto atmosphere = StE::Graphics::atmospherics_earth_properties({ 0,-6.371e+6,0 });

	StE::Graphics::atmospherics_precompute_scattering aps(atmosphere);

//	aps.load("atmospherics_scatter_lut.bin");

	aps.build_optical_length_lut();
	aps.build_scatter_lut(9);
	aps.build_ambient_lut();

	aps.write_out("atmospherics_scatter_lut.bin");

	return 0;
}
