// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture.h"

namespace StE {
namespace LLR {

class TextureCubeMap : public texture_mipmapped<llr_resource_type::LLRTextureCubeMap> {
private:
	using Base = texture_mipmapped<llr_resource_type::LLRTextureCubeMap>;

public:
	TextureCubeMap(TextureCubeMap &&m) = default;
	TextureCubeMap& operator=(TextureCubeMap &&m) = default;

	TextureCubeMap(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	TextureCubeMap(const gli::textureCube &t, bool generate_mipmaps = false) : texture_mipmapped(t.format(), t.dimensions(), generate_mipmaps ? calculate_mipmap_max_level(t[0].dimensions()) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::textureCube &t, bool generate_mipmaps = false);
	bool upload_face(LLRCubeMapFace face, const gli::texture2D &t);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		assert(face != LLRCubeMapFace::LLRCubeMapFaceNone && "face must be specified");
		auto gl_format = gl_utils::translate_format(format);

		bind();
		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTexSubImage2D(static_cast<GLenum>(face), static_cast<GLint>(level),
									  0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level),
									  gl_format.External,
									  data_size, data);
		}
		else {
			glTexSubImage2D(static_cast<GLenum>(face), static_cast<GLint>(level),
							0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level),
							gl_format.External, gl_format.Type,
							data);
		}
	}
};

}
}
