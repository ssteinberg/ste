
#include <stdafx.hpp>
#include <surface_mipmap_generator.hpp>

#include <gli/generate_mipmaps.hpp>
#include <gli/copy.hpp>

using namespace ste::resource;

gli::texture2d surface_mipmap_generator::operator()(const gli::texture2d &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture2d surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}
