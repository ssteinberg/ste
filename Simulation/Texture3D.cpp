
#include "stdafx.h"
#include "Texture3D.h"
#include "Log.h"

using namespace StE::LLR;

bool Texture3D::upload(const gli::texture3D &texture, bool gm) {
	int levels = gm ? 1 : std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.dimensions() != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	bind();
	for (std::size_t l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, 0, LLRCubeMapFace::LLRCubeMapFaceNone, texture[l].size());
	}

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return true;
}
