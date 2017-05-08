
#include <stdafx.hpp>
#include <surface_convert.hpp>

#include <gli/convert.hpp>

using namespace ste::resource;

gli::texture2d surface_convert::operator()(const gli::texture2d &input,
										   const gli::format &target_format) const {
	return gli::convert(input, target_format);
}
