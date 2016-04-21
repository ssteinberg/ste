// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

#include <algorithm>

namespace StE {
namespace Core {

class Texture2D : public texture_mipmapped<core_resource_type::Texture2D> {
private:
	using Base = texture_mipmapped<core_resource_type::Texture2D>;

public:
	Texture2D(Texture2D &&m) = default;
	Texture2D& operator=(Texture2D &&m) = default;

	Texture2D(gli::format format, const typename Base::size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture2D(const gli::texture2d &t, bool generate_mipmaps = false) :
				texture_mipmapped(t.format(),
								  typename Base::size_type{ t.extent().x, t.extent().y },
								  generate_mipmaps ? calculate_mipmap_max_level(typename Base::size_type{ t.extent().x, t.extent().y }) + 1 : t.levels(),
								  t.swizzles()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture2d &t, bool generate_mipmaps = false);

	void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) override {
		auto gl_format = GL::gl_utils::translate_format(format, Base::swizzle);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
										  0, 0, std::max(1, size[0] >> level), std::max(1, size[1] >> level),
										  gl_format.External,
										  data_size, data);
		}
		else {
			glTextureSubImage2D(get_resource_id(), static_cast<GLint>(level),
								0, 0, std::max(1, size[0] >> level), std::max(1, size[1] >> level),
								gl_format.External, gl_format.Type,
								data);
		}
	}

	const image<T> operator[](int level) const {
		return make_image(level);
	}
	const image<T> make_image(int level = 0) const {
		return image<T>(*this,
						get_image_container_size(),
						format, ImageAccessMode::ReadWrite,
						level, 0);
	}
};

}
}
