
#include <stdafx.hpp>
#include <surface_mipmap_generator.hpp>

#include <gli/generate_mipmaps.hpp>
#include <gli/copy.hpp>

using namespace ste::resource;

gli::texture1d surface_mipmap_generator::operator()(const gli::texture1d &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture1d surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture2d surface_mipmap_generator::operator()(const gli::texture2d &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture2d surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture3d surface_mipmap_generator::operator()(const gli::texture3d &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture3d surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture_cube surface_mipmap_generator::operator()(const gli::texture_cube &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture_cube surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture1d_array surface_mipmap_generator::operator()(const gli::texture1d_array &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture1d_array surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture2d_array surface_mipmap_generator::operator()(const gli::texture2d_array &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture2d_array surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}

gli::texture_cube_array surface_mipmap_generator::operator()(const gli::texture_cube_array &input) const {
	auto levels = gli::levels(input.extent());
	gli::texture_cube_array surface(input.format(), input.extent(), levels);
	gli::copy(input, surface);
	return gli::generate_mipmaps(surface, gli::FILTER_LINEAR);
}
