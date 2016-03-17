
#include "stdafx.hpp"
#include "TextureCubeMap.hpp"
#include "Log.hpp"

using namespace StE::LLR;

bool TextureCubeMap::upload(const gli::texture_cube &texture, bool gm) {
	bool ret = true;
	for (std::size_t i = 0; i < texture.faces(); ++i)
		ret &= this->upload_face(static_cast<LLRCubeMapFace>(i), texture[i]);

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return ret;
}

bool TextureCubeMap::upload_face(LLRCubeMapFace face, const gli::texture2d &texture) {
	gli::gl::format const format = gl_utils::translate_format(texture.format(), texture.swizzles());

	// Index of max level
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || decltype(this->size){ texture.extent().x, texture.extent().y } != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, 0, face, texture[l].size());
	}

	return true;
}
