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

#include "Sampler.h"

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
	static int allocate() { 
		GLuint id;
		glCreateTextures(opengl::gl_translate_type(type), 1, &id);
		return id;
	}
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
	mutable std::uint64_t tex_handle{ 0 };
	size_type size;
	gli::format format;

protected:
	auto get_image_container_size() const {
		glm::tvec2<std::size_t> s(1);
		for (int i = 0; i < std::min(2, dimensions()); ++i) s[i] = size[i];
		return s;
	}
	int get_image_container_dimensions() const { return dimensions() > 2 ? size[2] : get_layers(); }

	bool allocate_tex_storage(const size_type &size, gli::format gli_format, int levels, int samples, bool sparse, int page_size_idx = 0) {
		gli::gl::format const format = opengl::gl_translate_format(gli_format);

		if (sparse) {
			glTextureParameteri(id, GL_TEXTURE_SPARSE_ARB, true);
			glTextureParameteri(id, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, page_size_idx);
		}
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
	bool allocate_tex_storage(const size_type &size, gli::format gli_format, int levels, int samples, bool sparse, int page_size_idx, sampler_descriptor descriptor) {
		if (allocate_tex_storage(size, gli_format, levels, samples, sparse, page_size_idx)) {
			if (descriptor.wrap_s != TextureWrapMode::None) glTextureParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(descriptor.wrap_s));
			if (descriptor.wrap_t != TextureWrapMode::None) glTextureParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(descriptor.wrap_t));
			if (descriptor.wrap_r != TextureWrapMode::None) glTextureParameteri(id, GL_TEXTURE_WRAP_R, static_cast<GLenum>(descriptor.wrap_r));
			if (descriptor.min_mipmapping_filter()) glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(descriptor.min_mipmapping_filter()));
			if (descriptor.mag_filter != TextureFiltering::None) glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(descriptor.mag_filter));
			if (descriptor.anisotropy > 1.f) glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, descriptor.anisotropy);
			return true;
		}
		return false;
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
	auto get_image_size() const { return get_image_size(0); }
	gli::format get_format() const { return format; }
	int get_layers() const { return texture_is_array<type>::value ? this->size[texture_dimensions<type>::dimensions - 1] : 1; }

	bool is_compressed() const { return !!(gli::detail::getFormatInfo(format).Flags & gli::detail::CAP_COMPRESSED_BIT); }
	std::size_t get_storage_size(int level) const {
		std::size_t b = gli::block_size(format);
		int i;
		for (i = 0; i < texture_layer_dimensions<type>::dimensions; ++i) b *= std::max(1u, size[i] >> level);
		for (; i < dimensions(); ++i) b *= size[i];
		return b / (gli::block_dimensions_x(format) * gli::block_dimensions_y(format));
	}

	auto get_texture_handle() const {
		return tex_handle ? tex_handle : (tex_handle = glGetTextureHandleARB(id));
	}
	void make_resident() const { glMakeTextureHandleResidentARB(get_texture_handle()); }
	void make_nonresident() const { glMakeTextureHandleNonResidentARB(get_texture_handle()); }
	bool is_resident() const { return glIsTextureHandleResidentARB(get_texture_handle()); }

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
		allocate_tex_storage(size, format, 1, samples, false);
	}
	texture_multisampled(gli::format format, const size_type &size, int samples, sampler_descriptor descriptor) {
		this->samples = samples;
		allocate_tex_storage(size, format, 1, samples, false, 0, descriptor);
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
	virtual void download_level(void *data, std::size_t size, int level = 0, int layer = 0) const {
		auto &gl_format = opengl::gl_translate_format(format);
		glGetTextureImage(id, level, gl_format.External, gl_format.Type, size, data);
	}
	virtual void download_level(void *data, std::size_t size, int level, int layer, gli::format format, bool compressed = false) const {
		auto &gl_format = opengl::gl_translate_format(format);
		if (compressed)
			glGetCompressedTextureImage(id, level, size, data);
		else
			glGetTextureImage(id, level, gl_format.External, gl_format.Type, size, data);
	}
};

template <llr_resource_type type>
class texture_mipmapped : public texture_pixel_transferable<type> {
private:
	using Base = texture_pixel_transferable<type>;

protected:
	std::uint8_t levels;

	texture_mipmapped() {}
	texture_mipmapped(gli::format format, const size_type &size, int levels) : Base() {
		this->levels = levels;
		allocate_tex_storage(size, format, levels, 1, false);
	}
	texture_mipmapped(gli::format format, const size_type &size, int levels, sampler_descriptor descriptor) : Base() {
		this->levels = levels;
		allocate_tex_storage(size, format, levels, 1, false, 0, descriptor);
	}

public:
	texture_mipmapped(texture_mipmapped &&m) = default;
	texture_mipmapped& operator=(texture_mipmapped &&m) = default;

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
};

}
}
