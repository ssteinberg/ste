
#include "stdafx.h"
#include "Texture1DArray.h"
#include "Log.h"

using namespace StE::LLR;

bool Texture1DArray::upload(const gli::texture1DArray &texture, bool gm) {
	if (size_type({ texture.dimensions().x, texture.layers() }) != this->size) {
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

bool Texture1DArray::upload_layer(int layer, const gli::texture1D &texture) {
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.dimensions() != decltype(texture.dimensions())({ this->size.x })) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, layer, LLRCubeMapFace::LLRCubeMapFaceNone, texture[l].size());
	}

	return true;
}
