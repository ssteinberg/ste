// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace StE {
namespace Resource {

class surface_convert {
public:
	gli::texture2d operator()(const gli::texture2d &input,
							  const gli::format &target_format) const;
};

}
}
