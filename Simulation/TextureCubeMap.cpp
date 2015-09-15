
#include "stdafx.h"
#include "TextureCubeMap.h"
#include "Log.h"

using namespace StE::LLR;

bool TextureCubeMap::upload(const gli::textureCube &texture, bool gm) {
	bool ret = true;
	for (int i = 0; i < texture.faces(); ++i)
		ret &= this->upload_face(static_cast<LLRCubeMapFace>(i), texture[i]);

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return ret;
}

bool TextureCubeMap::upload_face(LLRCubeMapFace face, const gli::texture2D &texture) {
	gli::gl::format const format = opengl::gl_translate_format(texture.format());

	// Index of max level
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.dimensions() != this->size) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	bind();
	for (std::size_t l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, 0, face, texture[l].size());
	}

	return true;
}
