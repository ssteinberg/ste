// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "bindable_resource.h"
#include "PixelBufferObject.h"

#include "Log.h"

#include "texture_traits.h"

#include <type_traits>

namespace StE {
namespace LLR {

enum class LLRCubeMapFace {
	LLRCubeMapFaceNone = 0,
	LLRCubeMapFaceRight = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	LLRCubeMapFaceLeft = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	LLRCubeMapFaceTop = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	LLRCubeMapFaceBottom = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	LLRCubeMapFaceNear = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	LLRCubeMapFaceFar = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

template <llr_resource_type type>
class TextureAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateTextures(opengl::gl_translate_type(type), 1, &id); return id; }
	static void deallocate(unsigned int &id) { glDeleteTextures(1, reinterpret_cast<GLuint*>(&id)); id = 0; }
};

template <llr_resource_type type>
class TextureBinder {
private:
	constexpr static GLenum gl_type() { return opengl::gl_translate_type(type); }

public:
	static void bind(unsigned int id, int sampler = 0) {
		glActiveTexture(GL_TEXTURE0 + sampler); 
		glBindTexture(gl_type(), id);
	}
	static void unbind(int sampler = 0) { 
		glActiveTexture(GL_TEXTURE0 + sampler); 
		glBindTexture(gl_type(), 0);
	}
};

template <int dim> struct texture_size_type {};
template <> struct texture_size_type<1> { typedef glm::tvec1<std::size_t> type; };
template <> struct texture_size_type<2> { typedef glm::tvec2<std::size_t> type; };
template <> struct texture_size_type<3> { typedef glm::tvec3<std::size_t> type; };

class TextureGeneric : public bindable_generic_resource {
public:
	void bind() const override final { bind(0); }
	void unbind() const override final { unbind(0); }
	virtual void bind(int sampler) const = 0;
	virtual void unbind(int sampler) const = 0;
};

template <llr_resource_type type>
class Texture : public bindable_resource<TextureAllocator<type>, TextureBinder<type>, int>, public TextureGeneric {
private:
	using Base = bindable_resource<TextureAllocator<type>, TextureBinder<type>, int>;

	template <int dim, bool ms> static void create_gl_texture_storage(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<dim>::type &size) { static_assert(false); }
	template <> static void create_gl_texture_storage<1, false>(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<1>::type &size) {
		glTexStorage1D(gltype, levels, format.Internal, size[0]);
	}
	template <> static void create_gl_texture_storage<2, false>(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
		glTexStorage2D(gltype, levels, format.Internal, size[0], size[1]);
	}
	template <> static void create_gl_texture_storage<3, false>(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
		glTexStorage3D(gltype, levels, format.Internal, size[0], size[1], size[2]);
	}
	template <> static void create_gl_texture_storage<2, true>(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
		glTexStorage2DMultisample(gltype, samples, format.Internal, size[0], size[1], false);
	}
	template <> static void create_gl_texture_storage<3, true>(GLenum gltype, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
		glTexStorage3DMultisample(gltype, samples, format.Internal, size[0], size[1], size[2], false);
	}

public:
	using size_type = texture_size_type<texture_dimensions<type>::dimensions>::type;

protected:
	constexpr static GLenum gl_type() { return opengl::gl_translate_type(type); }

protected:
	size_type size;
	gli::format format;
	std::uint8_t levels, samples;

protected:
	bool allocate_tex_storage(const size_type &size, gli::format gli_format, int levels, int samples) {
		gli::gl::format const format = opengl::gl_translate_format(gli_format);

		bind();
		glTexParameteri(gl_type(), GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(gl_type(), GL_TEXTURE_MAX_LEVEL, levels - 1);
		glTexParameteri(gl_type(), GL_TEXTURE_SWIZZLE_R, format.Swizzle[0]);
		glTexParameteri(gl_type(), GL_TEXTURE_SWIZZLE_G, format.Swizzle[1]);
		glTexParameteri(gl_type(), GL_TEXTURE_SWIZZLE_B, format.Swizzle[2]);
		glTexParameteri(gl_type(), GL_TEXTURE_SWIZZLE_A, format.Swizzle[3]);

		create_gl_texture_storage<texture_dimensions<type>::dimensions, texture_is_multisampled<type>::value>(gl_type(), levels, samples, format, size);

		this->size = size;
		this->format = gli_format;
		this->levels = levels;
		this->samples = samples;

		return true;
	}

	Texture() {}

public:
	Texture(Texture &&m) = default;
	Texture& operator=(Texture &&m) = default;
	Texture(const Texture &m) = delete;
	Texture& operator=(const Texture &m) = delete;

	using TextureGeneric::bind;
	using TextureGeneric::unbind;
	void bind(int sampler) const override final { Base::bind(std::forward<int>(sampler)); }
	void unbind(int sampler) const override final { Base::unbind(std::forward<int>(sampler)); }

	void set_wrap_s(GLenum wrap_mode) { bind(); glTexParameteri(gl_type(), GL_TEXTURE_WRAP_S, wrap_mode); }
	void set_wrap_t(GLenum wrap_mode) { bind(); glTexParameteri(gl_type(), GL_TEXTURE_WRAP_T, wrap_mode); }
	void set_wrap_r(GLenum wrap_mode) { bind(); glTexParameteri(gl_type(), GL_TEXTURE_WRAP_R, wrap_mode); }

	static constexpr int dimensions() { return texture_dimensions<type>::dimensions; }
	static constexpr bool is_array_texture() { return texture_is_array<type>::value; }
	static constexpr bool is_multisampled() { return texture_is_multisampled<type>::value; }

	size_type get_size() const { return size; }
	gli::format get_format() const { return format; }
	int get_levels() { return levels; }
	int get_samples() { return samples; }
	int get_layers() { return texture_is_array<T>::value ? this->size[dimensions() - 1] : 1; }

	bool is_compressed() const { return !!(gli::detail::getFormatInfo(format).Flags & gli::detail::CAP_COMPRESSED_BIT); }
	std::size_t get_storage_size(int level) const {
		std::size_t b = gli::block_size(format);
		int i;
		for (i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) b *= std::max(1u, size[i] >> level);
		for (; i < dimensions(); ++i) b *= size[i];
		return b * samples / (gli::block_dimensions_x(format) * gli::block_dimensions_y(format));
	}
	std::size_t get_full_chain_storage_size() const {
		int t = 0;
		for (int l = 0; l < levels; ++l) t += get_storage_size(l);
		return t;
	}

	llr_resource_type resource_type() const override { return type; }
};

template <llr_resource_type type>
class texture_multisampled : public Texture<type> {
protected:
	texture_multisampled(gli::format format, const size_type &size, int samples) { allocate_tex_storage(size, format, 1, samples); }

public:
	texture_multisampled(texture_multisampled &&m) = default;
	texture_multisampled& operator=(texture_multisampled &&m) = default;
};

template <llr_resource_type type>
class texture_pixel_transferable : public Texture<type> {
protected:
	texture_pixel_transferable() {}
	texture_pixel_transferable(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable& operator=(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable(const texture_pixel_transferable &m) = delete;
	texture_pixel_transferable& operator=(const texture_pixel_transferable &m) = delete;

public:
	virtual void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) = 0;
	virtual void download_level(void *data, int level = 0, int layer = 0) const {
		auto &gl_format = opengl::gl_translate_format(format);
		if (is_compressed())
			glGetCompressedTexImage(gl_type(), level, data);
		else
			glGetTexImage(gl_type(), level, gl_format.External, gl_format.Type, data);
	}

	template <typename T, BufferUsage::buffer_usage U>
	void unpack_from(const PixelBufferObject<T, U> &pbo) { pbo.unpack_to(*this); }
	template <typename T, BufferUsage::buffer_usage U>
	void pack_to(PixelBufferObject<T, U> &pbo) const { pbo.pack_from(*this); }
};

template <llr_resource_type type>
class texture_mipmapped : public texture_pixel_transferable<type> {
protected:
	static int calculate_mipmap_max_level(const typename texture_size_type<texture_layer_dimensions<type>::dimensions>::type &size) {
		int levels = 0;
		for (int i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) {
			int l;
			auto x = size[i];
			for (l = 0; x >> l > 1; ++l);
			if (l > levels) levels = l;
		}
		return levels;
	}

	texture_mipmapped(gli::format format, const size_type &size, int levels) { allocate_tex_storage(size, format, levels, 1); }

public:
	texture_mipmapped(texture_mipmapped &&m) = default;
	texture_mipmapped& operator=(texture_mipmapped &&m) = default;
	texture_mipmapped(const texture_mipmapped &m) = delete;
	texture_mipmapped& operator=(const texture_mipmapped &m) = delete;

	void set_mag_filter(GLenum filter) { bind(); glTexParameteri(gl_type(), GL_TEXTURE_MAG_FILTER, filter); }
	void set_min_filter(GLenum filter) { bind(); glTexParameteri(gl_type(), GL_TEXTURE_MIN_FILTER, filter); }

	void generate_mipmaps() {
		bind();
		glGenerateMipmap(gl_type());
	}
};

}
}
