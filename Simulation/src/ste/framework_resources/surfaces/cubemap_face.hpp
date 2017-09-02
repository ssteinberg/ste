//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace resource {

enum class cubemap_face : std::uint32_t {
	x_positive = 0,
	x_negative = 1,
	y_positive = 2,
	y_negative = 3,
	z_positive = 4,
	z_negative = 5,

	right =	 x_positive,
	left =	 x_negative,
	top =	 y_positive,
	bottom = y_negative,
	front =	 z_positive,
	back =	 z_negative,
};

}
}
