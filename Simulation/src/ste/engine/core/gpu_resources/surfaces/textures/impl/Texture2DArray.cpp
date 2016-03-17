
#include "stdafx.hpp"
#include "Texture2DArray.hpp"
#include "Log.hpp"

using namespace StE::Core;

bool Texture2DArray::upload(const gli::texture2d_array &texture, bool gm) {
	if (size_type({ texture.extent().x, texture.extent().y, texture.layers() }) != this->size) {
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

bool Texture2DArray::upload_layer(int layer, const gli::texture2d &texture) {
	int levels = std::min(this->levels, static_cast<decltype(this->levels)>(texture.levels()));

	if (texture.format() != this->format || glm::ivec2(texture.extent().x, texture.extent().y) != glm::ivec2(this->size.x, this->size.y)) {
		ste_log_error() << "Texture format and size can not be changed!";
		assert(false);
		return false;
	}

	for (int l = 0; l < levels; ++l) {
		upload_level(texture[l].data(), l, layer, CubeMapFace::CubeMapFaceNone, texture[l].size());
	}

	return true;
}
