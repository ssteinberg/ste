// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
\
namespace ste {
namespace text {

namespace attributes {

enum class attrib_type : std::uint16_t {
	color,
	stroke,
	font,
	size,
	line_height,
	kern,
	align,
	weight,
	underline,
	italic,
	link,
};

}

}
}
