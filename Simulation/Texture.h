// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "Log.h"

#include "bindable_resource.h"
#include "layout_binding.h"

#include "RenderTarget.h"

#include "texture_enums.h"
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

class sampler_layout_binding_type {};
using sampler_layout_binding = layout_binding<sampler_layout_binding_type>;
sampler_layout_binding inline operator "" _sampler_idx(unsigned long long int i) { return sampler_layout_binding(i); }

template <llr_resource_type type>
class TextureAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateTextures(opengl::gl_translate_type(type), 1, &id); return id; }
	static void deallocate(unsigned int &id) { if (id) glDeleteTextures(1, reinterpret_cast<GLuint*>(&id)); id = 0; }
};

template <llr_resource_type type>
class TextureBinder {
private:
	constexpr static GLenum gl_type() { return opengl::gl_translate_type(type); }

public:
	static void bind(unsigned int id, const sampler_layout_binding &sampler) {
		glActiveTexture(GL_TEXTURE0 + sampler.binding_index()); 
		glBindTexture(gl_type(), id);
	}
	static void unbind(const sampler_layout_binding &sampler) {
		glActiveTexture(GL_TEXTURE0 + sampler.binding_index()); 
		glBindTexture(gl_type(), 0);
	}
};

template <int dim> struct texture_size_type {};
template <> struct texture_size_type<1> { using type = gli::storage::dim1_type; };
template <> struct texture_size_type<2> { using type = gli::storage::dim2_type; };
template <> struct texture_size_type<3> { using type = gli::storage::dim3_type; };

template <llr_resource_type type>
class texture : virtual public bindable_resource<TextureAllocator<type>, TextureBinder<type>, sampler_layout_binding>,
				virtual public shader_layout_bindable_resource<sampler_layout_binding_type> {
private:
	using Base = bindable_resource<TextureAllocator<type>, TextureBinder<type>, sampler_layout_binding>;

	template <int dim, bool ms> static void create_gl_texture_storage(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<dim>::type &size) { static_assert(false); }
	template <> static void create_gl_texture_storage<1, false>(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<1>::type &size) {
		glTextureStorage1D(id, levels, format.Internal, size[0]);
	}
	template <> static void create_gl_texture_storage<2, false>(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
		glTextureStorage2D(id, levels, format.Internal, size[0], size[1]);
	}
	template <> static void create_gl_texture_storage<3, false>(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
		glTextureStorage3D(id, levels, format.Internal, size[0], size[1], size[2]);
	}
	template <> static void create_gl_texture_storage<2, true>(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
		glTextureStorage2DMultisample(id, samples, format.Internal, size[0], size[1], false);
	}
	template <> static void create_gl_texture_storage<3, true>(int id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
		glTextureStorage3DMultisample(id, samples, format.Internal, size[0], size[1], size[2], false);
	}

public:
	using size_type = texture_size_type<texture_dimensions<type>::dimensions>::type;
	using image_size_type = texture_size_type<texture_layer_dimensions<type>::dimensions>::type;
	static constexpr llr_resource_type T = type;

protected:
	constexpr static GLenum gl_type() { return opengl::gl_translate_type(type); }

protected:
	size_type size;
	gli::format format;
	TextureFiltering mag_filter{ TextureFiltering::Linear };
	TextureFiltering min_filter{ TextureFiltering::Nearest };

private:
	template <llr_resource_type Temp = T>
	void set_size_from_alias_image_size(const image_size_type &image_size, int layers, std::enable_if_t<texture_is_array<Temp>::value>* = 0) { this->size = size_type(image_size, layers); }
	template <llr_resource_type Temp = T>
	void set_size_from_alias_image_size(const image_size_type &image_size, int layers, std::enable_if_t<!texture_is_array<Temp>::value>* = 0) { this->size = image_size; }

protected:
	auto get_image_container_size() const {
		glm::tvec2<std::size_t> s(1);
		for (int i = 0; i < std::min(2, dimensions()); ++i) s[i] = size[i];
		return s;
	}
	int get_image_container_dimensions() const { return dimensions() > 2 ? size[2] : get_layers(); }

	bool allocate_tex_storage(const size_type &size, gli::format gli_format, int levels, int samples) {
		assert((size.x % 4) == 0);

		gli::gl::format const format = opengl::gl_translate_format(gli_format);

		glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, levels - 1);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_R, format.Swizzle[0]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_G, format.Swizzle[1]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_B, format.Swizzle[2]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_A, format.Swizzle[3]);

		create_gl_texture_storage<texture_dimensions<type>::dimensions, texture_is_multisampled<type>::value>(id, levels, samples, format, size);

		this->size = size;
		this->format = gli_format;

		return true;
	}

	texture() {}

public:
	texture(texture &&m) = default;
	texture& operator=(texture &&m) = default;
	texture(const texture &m) = delete;
	texture& operator=(const texture &m) = delete;

	void bind() const { bind(LayoutLocationType(0)); }
	void unbind() const { unbind(LayoutLocationType(0)); }
	void bind(const LayoutLocationType &sampler) const final override { Base::bind(sampler); };
	void unbind(const LayoutLocationType &sampler) const final override { Base::unbind(sampler); };

	virtual void set_mag_filter(TextureFiltering filter) { mag_filter = filter; glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filter)); }
	virtual void set_min_filter(TextureFiltering filter) { min_filter = filter; glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter)); }
	TextureFiltering get_mag_filter() const { return mag_filter; }
	TextureFiltering get_min_filter() const { return min_filter; }

	void set_wrap_s(TextureWrapMode wrap_mode) { glTextureParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap_mode)); }
	void set_wrap_t(TextureWrapMode wrap_mode) { glTextureParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap_mode)); }
	void set_wrap_r(TextureWrapMode wrap_mode) { glTextureParameteri(id, GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrap_mode)); }

	int dimensions() const { return texture_dimensions<type>::dimensions; }
	bool is_array_texture() const { return texture_is_array<type>::value; }
	bool is_multisampled() const { return texture_is_multisampled<type>::value; }

	auto get_size() const { return size; }
	auto get_image_size(int level) const {
		image_size_type ret;
		for (int i = 0; i < texture_layer_dimensions<type>::dimensions; ++i)
			ret[i] = size[i] >> level;
		return ret;
	}
	auto get_image_size() const { return get_image_size(0, 0); }
	gli::format get_format() const { return format; }
	int get_layers() const { return texture_is_array<type>::value ? this->size[texture_dimensions<type>::dimensions - 1] : 1; }

	bool is_compressed() const { return !!(gli::detail::getFormatInfo(format).Flags & gli::detail::CAP_COMPRESSED_BIT); }
	std::size_t get_storage_size(int level) const {
		std::size_t b = gli::block_size(format);
		int i;
		for (i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) b *= std::max(1u, size[i] >> level);
		for (; i < dimensions(); ++i) b *= size[i];
		return b * samples / (gli::block_dimensions_x(format) * gli::block_dimensions_y(format));
	}

	llr_resource_type resource_type() const override { return type; }
};

template <llr_resource_type type>
class texture_multisampled : public texture<type> {
private:
	using Base = texture<type>;

protected:
	std::uint8_t samples;

	texture_multisampled(gli::format format, const size_type &size, int samples) {
		this->samples = samples;
		allocate_tex_storage(size, format, 1, samples);
	}

public:
	texture_multisampled(texture_multisampled &&m) = default;
	texture_multisampled& operator=(texture_multisampled &&m) = default;

	int get_samples() const { return samples; }
};

template <llr_resource_type type>
class texture_pixel_transferable : public texture<type> {
protected:
	using texture<type>::texture;

	texture_pixel_transferable() {}

	texture_pixel_transferable(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable& operator=(texture_pixel_transferable &&m) = default;
	texture_pixel_transferable(const texture_pixel_transferable &m) = delete;
	texture_pixel_transferable& operator=(const texture_pixel_transferable &m) = delete;

public:
	virtual void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) = 0;
	virtual void download_level(void *data, int level = 0, int layer = 0) const {
		auto &gl_format = opengl::gl_translate_format(format);
		bind();
		glGetTexImage(gl_type(), level, gl_format.External, gl_format.Type, data);
	}
	virtual void download_level(void *data, int level, int layer, gli::format format, bool compressed = false) const {
		auto &gl_format = opengl::gl_translate_format(format);
		bind();
		if (compressed)
			glGetCompressedTexImage(gl_type(), level, data);
		else
			glGetTexImage(gl_type(), level, gl_format.External, gl_format.Type, data);
	}
};

template <llr_resource_type type>
class texture_mipmapped : public texture_pixel_transferable<type> {
private:
	using Base = texture_pixel_transferable<type>;

	TextureFiltering mipmap_filter{ TextureFiltering::Linear };

	GLenum min_mipmapping_filter() {
		if (mipmap_filter == TextureFiltering::Nearest && min_filter == TextureFiltering::Nearest)	return GL_NEAREST_MIPMAP_NEAREST;
		if (mipmap_filter == TextureFiltering::Linear && min_filter == TextureFiltering::Nearest)	return GL_NEAREST_MIPMAP_LINEAR;
		if (mipmap_filter == TextureFiltering::Nearest && min_filter == TextureFiltering::Linear)	return GL_LINEAR_MIPMAP_NEAREST;
		return GL_LINEAR_MIPMAP_LINEAR;
	}

protected:
	std::uint8_t levels;

	static int calculate_mipmap_max_level(const typename texture_size_type<texture_layer_dimensions<type>::dimensions>::type &size) {
		int levels = 0;
		for (int i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) {
			int l;
			auto x = size[i];
			for (l = 0; x >> l > 1; ++l);
			levels = std::max(l, levels);
		}
		return levels;
	}

	texture_mipmapped(gli::format format, const size_type &size, int levels) : Base() {
		this->levels = levels;
		allocate_tex_storage(size, format, levels, 1);
	}

public:
	texture_mipmapped(texture_mipmapped &&m) = default;
	texture_mipmapped& operator=(texture_mipmapped &&m) = default;

	void set_min_filter(TextureFiltering filter) override final { min_filter = filter; glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_mipmapping_filter()); }
	void set_mipmap_filter(TextureFiltering filter) { mipmap_filter = filter; glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_mipmapping_filter()); }
	TextureFiltering get_mipmap_filter() const { return mipmap_filter; }
	int get_levels() const { return levels; }

	void generate_mipmaps() {
		bind();
		glGenerateMipmap(gl_type());
	}

	std::size_t get_full_chain_storage_size() const {
		int t = 0;
		for (int l = 0; l < levels; ++l) t += get_storage_size(l);
		return t;
	}
};

}
}
