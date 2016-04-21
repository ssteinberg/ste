// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "texture_base.hpp"
#include "image.hpp"

namespace StE {
namespace Core {

class Texture1DArray : public texture_mipmapped<core_resource_type::Texture1DArray> {
private:
	using Base = texture_mipmapped<core_resource_type::Texture1DArray>;

public:
	Texture1DArray(Texture1DArray &&m) = default;
	Texture1DArray& operator=(Texture1DArray &&m) = default;

	Texture1DArray(gli::format format, const typename Base::size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	Texture1DArray(const gli::texture1d_array &t, bool generate_mipmaps = false)
		: texture_mipmapped(t.format(), typename Base::size_type({ t.extent().x, t.layers() }),
							generate_mipmaps ? calculate_mipmap_max_level(typename texture_size_type<1>::type{ t.extent().x }) + 1 : t.levels(),
							t.swizzles()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture1d_array &t, bool generate_mipmaps = false);
	bool upload_layer(int layer, const gli::texture1d &t);

	void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) override {
		auto gl_format = GL::gl_utils::translate_format(format, swizzle);

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

	using Base::download_level;
	void download_level(void *data,
						std::size_t size,
						int level,
						int layer) const {
		auto gl_format = GL::gl_utils::translate_format(Base::format, Base::swizzle);

		glGetTextureSubImage(get_resource_id(), level,
							 0, layer, 0,
							 Base::get_size().x, 1, 0,
							 gl_format.External, gl_format.Type, size, data);
	}
	void download_level(void *data,
						std::size_t size,
						int level,
						int layer,
						const gli::format &format,
						const gli::swizzles &swizzle = swizzles_rgba,
						bool compressed = false) const {
		auto gl_format = GL::gl_utils::translate_format(format, swizzle);

		if (compressed)
			glGetCompressedTextureSubImage(get_resource_id(), level,
										   0, layer, 0,
										   Base::get_size().x, 1, 0,
										   size, data);
		else
			glGetTextureSubImage(get_resource_id(), level,
								 0, layer, 0,
								 Base::get_size().x, 1, 0,
								 gl_format.External, gl_format.Type, size, data);
	}

	const image<T> operator[](int layer) const {
		return make_image(layer, 0);
	}
	const image_container<T> make_image(int level = 0) const {
		return image_container<T>(*this,
								  get_image_container_size(),
								  format, ImageAccessMode::ReadWrite,
								  level, get_layers());
	}
	const image<T> make_image(int layer, int level) const {
		return image<T>(*this,
						get_image_container_size(),
						format, ImageAccessMode::ReadWrite,
						level, layer);
	}
};

}
}
