
#include "stdafx.hpp"
#include "Texture2D.hpp"
#include "Log.hpp"

using namespace StE::LLR;

bool Texture2D::upload(const gli::texture2d &texture, bool gm) {
	int levels = gm ? 1 : std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format) {
		ste_log_error() << "Texture format can not be changed!";
		assert(false);
		return false;
	}
	auto new_size = decltype(this->size){ texture.extent().x, texture.extent().y };
	if (new_size != this->size) {
		ste_log_error() << "Texture size can not be changed! Was " << std::to_string(this->size.x) << " X " << std::to_string(this->size.y) << ", uploading texture of size " << new_size.x << " X " << new_size.y << "." << std::endl;
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
