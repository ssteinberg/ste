// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture.h"
#include "image.h"

namespace StE {
namespace LLR {

class Texture2DArray : public texture_mipmapped<llr_resource_type::LLRTexture2DArray> {
private:
	using Base = texture_mipmapped<llr_resource_type::LLRTexture2DArray>;

public:
	Texture2DArray(Texture2DArray &&m) = default;
	Texture2DArray& operator=(Texture2DArray &&m) = default;

	Texture2DArray(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture2DArray(const gli::texture2DArray &t, bool generate_mipmaps = false) 
		: texture_mipmapped(t.format(), size_type({ t.dimensions().xy, t.layers() }), generate_mipmaps ? calculate_mipmap_max_level(t.dimensions().xy) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture2DArray &t, bool generate_mipmaps = false);
	bool upload_layer(int layer, const gli::texture2D &t);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		auto &gl_format = opengl::gl_translate_format(format);

		bind();
		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTexSubImage3D(gl_type(), static_cast<GLint>(level),
									  0, 0, layer, 
									  std::max(1u, size[0] >> level), std::max(1u, size[1] >> level), 1,
									  gl_format.External,
									  data_size, data);
		}
		else {
			glTexSubImage3D(gl_type(), static_cast<GLint>(level),
							0, 0, layer, 
							std::max(1u, size[0] >> level), std::max(1u, size[1] >> level), 1,
							gl_format.External, gl_format.Type,
							data);
		}
	}

	const image_container<T> operator[](int level) const {
		return image_container<T>(id, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, get_image_container_dimensions());
	}
};

}
}
