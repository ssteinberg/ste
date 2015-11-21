// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gl_utils.h"

#include "Log.h"
#include "gl_current_context.h"

#include "bindable_resource.h"
#include "layout_binding.h"

#include "RenderTarget.h"
#include "Sampler.h"

#include "texture_handle.h"
#include "texture_enums.h"
#include "texture_traits.h"
#include "texture_allocator.h"

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

class texture_layout_binding_type {};
using texture_layout_binding = layout_binding<texture_layout_binding_type>;
texture_layout_binding inline operator "" _tex_unit(unsigned long long int i) { return texture_layout_binding(i); }

template <llr_resource_type type>
class TextureBinder {
private:
	static constexpr GLenum gl_type() { return gl_utils::translate_type(type); }

public:
	static void bind(unsigned int id, const texture_layout_binding &sampler) {
		gl_current_context::get()->bind_texture_unit(sampler.binding_index(), id);
	}
	static void unbind(const texture_layout_binding &sampler) {
		gl_current_context::get()->bind_texture_unit(sampler.binding_index(), 0);
	}
};

template <llr_resource_type type>
class texture : virtual public bindable_resource<texture_immutable_storage_allocator<type>, TextureBinder<type>, texture_layout_binding>,
				virtual public shader_layout_bindable_resource<texture_layout_binding_type> {
private:
	using Base = bindable_resource<texture_immutable_storage_allocator<type>, TextureBinder<type>, texture_layout_binding>;
	
public:
	using size_type = texture_size_type<texture_dimensions<type>::dimensions>::type;
	using image_size_type = texture_size_type<texture_dimensions<type>::dimensions>::type;
	static constexpr llr_resource_type T = type;

protected:
	static constexpr GLenum gl_type() { return gl_utils::translate_type(type); }

	int levels, samples;
	texture_size_type<texture_dimensions<type>::dimensions>::type size;
	gli::format format;

protected:
	auto get_image_container_size() const {
		glm::tvec2<std::size_t> s(1);
		for (int i = 0; i < std::min(2, dimensions()); ++i) s[i] = size[i];
		return s;
	}
	int get_image_container_dimensions() const { return dimensions() > 2 ? size[2] : get_layers(); }

	bool allocate_tex_storage(const size_type &size, gli::format gli_format, int levels, int samples, bool sparse, int page_size_idx = 0) {
		gli::gl::format const glformat = gl_utils::translate_format(gli_format);

		auto id = get_resource_id();
		if (sparse) {
			glTextureParameteri(id, GL_TEXTURE_SPARSE_ARB, true);
			glTextureParameteri(id, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, page_size_idx);
		}
		glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, levels - 1);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_R, glformat.Swizzle[0]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_G, glformat.Swizzle[1]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_B, glformat.Swizzle[2]);
		glTextureParameteri(id, GL_TEXTURE_SWIZZLE_A, glformat.Swizzle[3]);

		this->format = gli_format;
		this->size = size;
		this->levels = levels;
		this->size = size;

		allocator.allocate_storage(get_resource_id(), levels, samples, glformat, size);

		return true;
	}

	texture() {}

public:
	texture(texture &&m) = default;
	texture& operator=(texture &&m) = default;
	texture(const texture &m) = delete;
	texture& operator=(const texture &m) = delete;

	virtual ~texture() noexcept {}

	void bind() const { bind(LayoutLocationType(0)); }
	void unbind() const { unbind(LayoutLocationType(0)); }
	void bind(const LayoutLocationType &sampler) const final override { Base::bind(sampler); };
	void unbind(const LayoutLocationType &sampler) const final override { Base::unbind(sampler); };

	constexpr int dimensions() const { return texture_dimensions<type>::dimensions; }
	constexpr bool is_array_texture() const { return texture_is_array<type>::value; }
	constexpr bool is_multisampled() const { return texture_is_multisampled<type>::value; }

	void clear(void *data, int level = 0) {
		gli::gl::format format = gl_utils::translate_format(format);
		glClearTexImage(get_resource_id(), level, format.External, format.Type, data);
	}

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
		return texture_handle(glGetTextureHandleARB(get_resource_id()));
	}
	auto get_texture_handle(const Sampler &sam) const {
		unsigned sam_id = sam.get_resource_id();
		return texture_handle(glGetTextureSamplerHandleARB(get_resource_id(), sam_id));
	}

	llr_resource_type resource_type() const override { return type; }
};

template <llr_resource_type type>
class texture_multisampled : public texture<type> {
private:
	using Base = texture<type>;

protected:
	texture_multisampled(gli::format format, const size_type &size, int samples) {
		allocate_tex_storage(size, format, 1, samples, false);
	}

public:
	texture_multisampled(texture_multisampled &&m) = default;
	texture_multisampled& operator=(texture_multisampled &&m) = default;

	virtual ~texture_multisampled() noexcept {}

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
	virtual ~texture_pixel_transferable() noexcept {}

	virtual void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) = 0;
	virtual void download_level(void *data, std::size_t size, int level = 0, int layer = 0) const {
		auto gl_format = gl_utils::translate_format(format);
		glGetTextureImage(get_resource_id(), level, gl_format.External, gl_format.Type, size, data);
	}
	virtual void download_level(void *data, std::size_t size, int level, int layer, gli::format format, bool compressed = false) const {
		auto gl_format = gl_utils::translate_format(format);
		if (compressed)
			glGetCompressedTextureImage(get_resource_id(), level, size, data);
		else
			glGetTextureImage(get_resource_id(), level, gl_format.External, gl_format.Type, size, data);
	}
};

template <llr_resource_type type>
class texture_mipmapped : public texture_pixel_transferable<type> {
private:
	using Base = texture_pixel_transferable<type>;

protected:
	texture_mipmapped() {}
	texture_mipmapped(gli::format format, const size_type &size, int levels) : Base() {
		allocate_tex_storage(size, format, levels, 1, false);
	}

public:
	texture_mipmapped(texture_mipmapped &&m) = default;
	texture_mipmapped& operator=(texture_mipmapped &&m) = default;

	virtual ~texture_mipmapped() noexcept {}

	int get_levels() const { return levels; }

	void generate_mipmaps() {
		bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(gl_type(), get_resource_id());
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
