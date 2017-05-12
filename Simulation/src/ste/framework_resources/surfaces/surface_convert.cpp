
#include <stdafx.hpp>
#include <surface_convert.hpp>

#include <gli/convert.hpp>

using namespace ste::resource;

gli::texture1d surface_convert::operator()(const gli::texture1d &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture2d surface_convert::operator()(const gli::texture2d &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture3d surface_convert::operator()(const gli::texture3d &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture_cube surface_convert::operator()(const gli::texture_cube &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture1d_array surface_convert::operator()(const gli::texture1d_array &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture2d_array surface_convert::operator()(const gli::texture2d_array &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

gli::texture_cube_array surface_convert::operator()(const gli::texture_cube_array &input,
						  const gli::format &target_format) const {
	return gli::convert(input, target_format);
}

