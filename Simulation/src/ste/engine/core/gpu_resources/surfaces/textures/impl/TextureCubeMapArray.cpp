
#include "stdafx.hpp"
#include "TextureCubeMapArray.hpp"
#include "Log.hpp"

using namespace StE::Core;

bool TextureCubeMapArray::upload(const gli::texture_cube_array &texture, bool gm) {
	bool ret = true;
	for (std::size_t i = 0; i < texture.layers(); ++i)
		ret &= this->upload_layer(i, texture[i]);

	// Generate mipmaps
	if (gm) generate_mipmaps();

	return ret;
}

bool TextureCubeMapArray::upload_layer(int layer, const gli::texture_cube &texture) {
	bool ret = true;
	for (std::size_t i = 0; i < texture.faces(); ++i)
		ret &= this->upload_face(static_cast<CubeMapFace>(i), layer, texture[i]);

	return ret;
}

bool TextureCubeMapArray::upload_face(CubeMapFace face, int layer, const gli::texture2d &texture) {
	gli::gl::format const format = gl_utils::translate_format(texture.format(), texture.swizzles());

	// Index of max level
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || texture.extent() != glm::ivec2(this->size.x, this->size.y)) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, layer, face, texture[l].size());
	}

	return true;
}
