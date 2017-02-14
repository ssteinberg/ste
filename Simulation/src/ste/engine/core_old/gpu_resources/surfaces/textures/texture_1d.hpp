// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <texture_base.hpp>
#include <image.hpp>

namespace StE {
namespace Core {

class texture_1d : public texture_mipmapped<core_resource_type::Texture1D> {
private:
	using Base = texture_mipmapped<core_resource_type::Texture1D>;

public:
	texture_1d(texture_1d &&m) = default;
	texture_1d& operator=(texture_1d &&m) = default;

	texture_1d(gli::format format, const Base::size_type &size, int levels = 1) : texture_mipmapped(format, size, levels) {}
	texture_1d(const gli::texture1d &t, bool generate_mipmaps = false) :
				texture_mipmapped(t.format(),
								  Base::size_type{ t.extent().x },
								  generate_mipmaps ? calculate_mipmap_max_level(Base::size_type{ t.extent().x }) + 1 : t.levels(),
								  t.swizzles()) {
		upload(t, generate_mipmaps);
	}

	// Re upload texture data. Surface must match texture's format.
	bool upload(const gli::texture1d &t, bool generate_mipmaps = false);

	void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) override {
		auto gl_format = GL::gl_utils::translate_format(format, swizzle);

		if (is_compressed()) {
			assert(data_size && "size must be specified for compressed levels");
			glCompressedTextureSubImage1D(get_resource_id(), static_cast<GLint>(level),
										  0, std::max(1, size[0] >> level),
										  gl_format.External,
										  data_size, data);
		}
		else {
			glTextureSubImage1D(get_resource_id(), static_cast<GLint>(level),
								0, std::max(1, size[0] >> level),
								gl_format.External, gl_format.Type,
								data);
		}
	}

	image<T> operator[](int level) const {
		return make_image(level);
	}
	image<T> make_image(int level = 0) const {
		return image<T>(*this,
						get_image_container_size(),
						format, image_access_mode::ReadWrite,
						level, 0);
	}
};

}
}
