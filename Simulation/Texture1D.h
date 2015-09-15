// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "Texture.h"

namespace StE {
namespace LLR {

class Texture1D : public texture_mipmapped<llr_resource_type::LLRTexture1D> {
public:
	Texture1D(Texture1D &&m) = default;
	Texture1D& operator=(Texture1D &&m) = default;

	Texture1D(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture1D(const gli::texture1D &t, bool generate_mipmaps = false) : texture_mipmapped(t.format(), t.dimensions(), generate_mipmaps ? calculate_mipmap_max_level(t.dimensions()) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture1D &t, bool generate_mipmaps = false);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		auto &gl_format = opengl::gl_translate_format(format);
		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTexSubImage1D(gl_type(), static_cast<GLint>(level),
									  0, std::max(1u, size[0] >> level),
									  gl_format.External,
									  data_size, data);
		}
		else {
			glTexSubImage1D(gl_type(), static_cast<GLint>(level),
							0, std::max(1u, size[0] >> level),
							gl_format.External, gl_format.Type,
							data);
		}
	}
};

}
}
