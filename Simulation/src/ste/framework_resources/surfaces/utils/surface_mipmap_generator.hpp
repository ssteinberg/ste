// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace resource {

class surface_mipmap_generator {
public:
	gli::texture1d operator()(const gli::texture1d &input) const;
	gli::texture2d operator()(const gli::texture2d &input) const;
	gli::texture3d operator()(const gli::texture3d &input) const;
	gli::texture_cube operator()(const gli::texture_cube &input) const;

	gli::texture1d_array operator()(const gli::texture1d_array &input) const;
	gli::texture2d_array operator()(const gli::texture2d_array &input) const;
	gli::texture_cube_array operator()(const gli::texture_cube_array &input) const;
};

}
}
