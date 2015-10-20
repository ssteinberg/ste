// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture.h"
#include "image.h"

namespace StE {
namespace LLR {

class Texture3D : public texture_mipmapped<llr_resource_type::LLRTexture3D> {
private:
	using Base = texture_mipmapped<llr_resource_type::LLRTexture3D>;

public:
	Texture3D(Texture3D &&m) = default;
	Texture3D& operator=(Texture3D &&m) = default;

	Texture3D(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture3D(const gli::texture3D &t, bool generate_mipmaps = false) : texture_mipmapped(t.format(), t.dimensions(), generate_mipmaps ? calculate_mipmap_max_level(t.dimensions()) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture3D &t, bool generate_mipmaps = false);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		auto gl_format = gl_utils::translate_format(format);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage3D(get_resource_id(), static_cast<GLint>(level),
										  0, 0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level), std::max(1u, size[2] >> level),
										  gl_format.External,
										  data_size, data);
		}
		else {
			glTextureSubImage3D(get_resource_id(), static_cast<GLint>(level),
								0, 0, 0, std::max(1u, size[0] >> level), std::max(1u, size[1] >> level), std::max(1u, size[2] >> level),
								gl_format.External, gl_format.Type,
								data);
		}
	}

	const image_container<T> operator[](int level) const {
		return image_container<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, level, get_image_container_dimensions());
	}
};

}
}
