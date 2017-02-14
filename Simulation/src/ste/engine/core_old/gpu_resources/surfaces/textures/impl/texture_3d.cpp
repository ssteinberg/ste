
#include <stdafx.hpp>
#include <texture_3d.hpp>
#include <Log.hpp>

using namespace StE::Core;

bool texture_3d::upload(const gli::texture3d &texture, bool gm) {
	int levels = gm ? 1 : std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.extent() != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, 0, CubeMapFace::CubeMapFaceNone, texture[l].size());
	}

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return true;
}
