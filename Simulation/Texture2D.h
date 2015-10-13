// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture.h"
#include "image.h"

#include <algorithm>

namespace StE {
namespace LLR {

class Texture2D : public texture_mipmapped<llr_resource_type::LLRTexture2D> {
private:
	using Base = texture_mipmapped<llr_resource_type::LLRTexture2D>;

public:
	Texture2D(Texture2D &&m) = default;
	Texture2D& operator=(Texture2D &&m) = default;

	Texture2D(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture2D(gli::format format, const size_type &size, int levels, sampler_descriptor descriptor) : texture_mipmapped(format, size, levels, descriptor) {}
	Texture2D(const gli::texture2D &t, bool generate_mipmaps = false) : texture_mipmapped(t.format(), t.dimensions(), generate_mipmaps ? calculate_mipmap_max_level(t.dimensions()) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture2D &t, bool generate_mipmaps = false);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		auto &gl_format = opengl::gl_translate_format(format);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
										  0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level),
										  gl_format.External,
										  data_size, data);
		}
		else {
			glTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
								0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level),
								gl_format.External, gl_format.Type,
								data);
		}
	}

	const image<T> operator[](int level) const {
		return image<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, 0);
	}
};

}
}
