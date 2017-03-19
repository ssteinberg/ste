// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace StE {
namespace Resource {

class surface_mipmap_generator {
public:
	gli::texture2d operator()(const gli::texture2d &input) const;
};

}
}
