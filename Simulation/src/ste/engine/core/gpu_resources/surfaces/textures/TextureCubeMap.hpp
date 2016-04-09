// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

#include "texture_cubemap_face.hpp"

namespace StE {
namespace Core {

class TextureCubeMap : public texture_mipmapped<core_resource_type::TextureCubeMap> {
private:
	using Base = texture_mipmapped<core_resource_type::TextureCubeMap>;

public:
	TextureCubeMap(TextureCubeMap &&m) = default;
	TextureCubeMap& operator=(TextureCubeMap &&m) = default;

	TextureCubeMap(gli::format format, const typename Base::size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	TextureCubeMap(const gli::texture_cube &t, bool generate_mipmaps = false) :
						texture_mipmapped(t.format(),
										  t.extent(),
										  generate_mipmaps ? calculate_mipmap_max_level(t[0].extent()) + 1 : t.levels(),
										  t.swizzles()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture_cube &t, bool generate_mipmaps = false);
	bool upload_face(CubeMapFace face, const gli::texture2d &t);

	void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) override {
		assert(face != CubeMapFace::CubeMapFaceNone && "face must be specified");
		auto gl_format = gl_utils::translate_format(format, swizzle);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage2D(static_cast<GLenum>(face), static_cast<GLint>(level),
										  0, 0, std::max(1, size[0] >> level), std::max(1, size[1] >> level),
										  gl_format.External,
									  	data_size, data);
		}
		else {
			glTextureSubImage2D(static_cast<GLenum>(face), static_cast<GLint>(level),
								0, 0, std::max(1, size[0] >> level), std::max(1, size[1] >> level),
								gl_format.External, gl_format.Type,
								data);
		}
	}

	using Base::download_level;
	void download_level(void *data,
						std::size_t size,
						int level,
						CubeMapFace face) const {
		auto gl_format = gl_utils::translate_format(Base::format, Base::swizzle);

		bind();
		glGetTexImage(static_cast<GLenum>(face), level, gl_format.External, gl_format.Type, data);
	}
	void download_level(void *data,
						std::size_t size,
						int level,
						CubeMapFace face,
						const gli::format &format,
						const gli::swizzles &swizzle = swizzles_rgba,
						bool compressed = false) const {
		auto gl_format = gl_utils::translate_format(format, swizzle);

		bind();
		if (compressed)
			glGetCompressedTexImage(static_cast<GLenum>(face), level, data);
		else
			glGetTexImage(static_cast<GLenum>(face), level, gl_format.External, gl_format.Type, data);
	}

	const image<T> operator[](CubeMapFace face) const {
		return image<T>(*this, get_image_container_size(), format, ImageAccessMode::ReadWrite, 0, static_cast<int>(face) - static_cast<int>(CubeMapFace::CubeMapFaceRight));
	}
};

}
}
