// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "texture_base.h"
#include "image.h"

namespace StE {
namespace LLR {

class Texture1DArray : public texture_mipmapped<llr_resource_type::LLRTexture1DArray> {
private:
	using Base = texture_mipmapped<llr_resource_type::LLRTexture1DArray>;
	
public:
	Texture1DArray(Texture1DArray &&m) = default;
	Texture1DArray& operator=(Texture1DArray &&m) = default;

	Texture1DArray(gli::format format, const size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture1DArray(const gli::texture1d_array &t, bool generate_mipmaps = false)
		: texture_mipmapped(t.format(), typename Base::size_type({ t.extent().x, t.layers() }), 
							generate_mipmaps ? calculate_mipmap_max_level(typename texture_size_type<1>::type{ t.extent().x }) + 1 : t.levels()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture1d_array &t, bool generate_mipmaps = false);
	bool upload_layer(int layer, const gli::texture1d &t);

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {
		auto gl_format = gl_utils::translate_format(format);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
										  0, layer,
										  std::max(1, size[0] >> level), 1,
										  gl_format.External,
										  data_size, data);
		}
		else {
			glTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
								0, layer,
								std::max(1, size[0] >> level), 1,
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
