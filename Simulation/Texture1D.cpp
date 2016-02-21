
#include "stdafx.h"
#include "Texture1D.h"
#include "Log.h"

using namespace StE::LLR;

bool Texture1D::upload(const gli::texture1d &texture, bool gm) {
	int levels = gm ? 1 : std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.extent().x != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, 0, LLRCubeMapFace::LLRCubeMapFaceNone, texture[l].size());
	}

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return true;
}
