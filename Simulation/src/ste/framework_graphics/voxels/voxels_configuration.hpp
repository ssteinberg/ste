//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

struct voxels_configuration {
	// (2^P)^3 voxels per block
	std::uint32_t P = 2;
	// (2^Pi)^3 voxels per initial block
	std::uint32_t Pi = 4;
	// Voxel structure end level index
	std::uint32_t leaf_level = 5;

	// Voxel world size
	float world = 1000;
};

}
}
