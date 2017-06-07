// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
\
namespace StE {
namespace Text {

namespace Attributes {

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