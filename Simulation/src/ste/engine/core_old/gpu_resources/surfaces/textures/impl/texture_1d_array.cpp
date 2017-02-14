
#include <stdafx.hpp>
#include <texture_1d_array.hpp>
#include <Log.hpp>

using namespace StE::Core;

bool texture_1d_array::upload(const gli::texture1d_array &texture, bool gm) {
	if (size_type({ texture.extent().x, texture.layers() }) != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	bool ret = true;
	for (std::size_t i = 0; i < texture.layers(); ++i)
		ret &= this->upload_layer(i, texture[i]);

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return true;
}

bool texture_1d_array::upload_layer(int layer, const gli::texture1d &texture) {
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.extent().x != this->size.x) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, layer, CubeMapFace::CubeMapFaceNone, texture[l].size());
	}

	return true;
}
