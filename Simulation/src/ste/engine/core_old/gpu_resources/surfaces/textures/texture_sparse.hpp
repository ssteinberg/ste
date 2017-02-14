// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <texture_base.hpp>
#include <core_resource_type.hpp>
#include <surface_constants.hpp>

#include <image.hpp>

#include <vector>

namespace StE {
namespace Core {

template <core_resource_type TextureType>
class texture_sparse_mipmapped : public texture_mipmapped<TextureType> {
private:
	using Base = texture_mipmapped<TextureType>;

	void upload_level(const void *data, int level = 0, int layer = 0, CubeMapFace face = CubeMapFace::CubeMapFaceNone, int data_size = 0) override {}

protected:
	typename Base::size_type tile_size;
	int max_sparse_level;

	texture_sparse_mipmapped(gli::format format,
							 const typename Base::size_type &size,
							 int levels,
							 const typename Base::size_type &tile_size,
							 int virtual_page_idx,
							 const gli::swizzles &swizzle = swizzles_rgba) : Base(), tile_size(tile_size) {
		this->levels = levels;
		Base::allocate_tex_storage(size, format, swizzle, levels, 1, true, virtual_page_idx);
		glGetTextureParameteriv(Base::get_resource_id(), GL_NUM_SPARSE_LEVELS_ARB, &max_sparse_level);
	}

public:
	texture_sparse_mipmapped(texture_sparse_mipmapped &&m) = default;
	texture_sparse_mipmapped& operator=(texture_sparse_mipmapped &&m) = default;

	auto get_tile_size() const { return tile_size; }
	auto get_max_sparse_level() const { return max_sparse_level; }

	void commit_tiles(const typename Base::size_type &offset_size, const typename Base::size_type &count_size, int level) {
		glm::tvec3<std::size_t> offset3{ 0,0,0 };
		glm::tvec3<std::size_t> count3{ 1,1,1 };
		for (int i = 0; i < Base::dimensions(); ++i) {
			offset3[i] = offset_size[i];
			count3[i] = count_size[i];
		}

		glTexturePageCommitmentEXT(Base::get_resource_id(), level, offset3.x, offset3.y, offset3.z, count3.x, count3.y, count3.z, true);
	}
	void uncommit_tiles(const typename Base::size_type &offset_size, const typename Base::size_type &count_size, int level) {
		glm::tvec3<std::size_t> offset3{ 0,0,0 };
		glm::tvec3<std::size_t> count3{ 1,1,1 };
		for (int i = 0; i < Base::dimensions(); ++i) {
			offset3[i] = offset_size[i];
			count3[i] = count_size[i];
		}

		glTexturePageCommitmentEXT(Base::get_resource_id(), level, offset3.x, offset3.y, offset3.z, count3.x, count3.y, count3.z, false);
	}

	static std::vector<glm::ivec3> page_sizes(gli::format gli_format, const gli::swizzles &swizzle = swizzles_rgba) {
		gli::gl::format const format = GL::gl_utils::translate_format(gli_format, swizzle);

		int n;
		glGetInternalformativ(Base::gl_type(), format.External, GL_NUM_VIRTUAL_PAGE_SIZES_ARB, 1, &n);
		std::vector<int> x(n), y(n), z(n);
		glGetInternalformativ(Base::gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_X_ARB, n*sizeof(int), &(x[0]));
		glGetInternalformativ(Base::gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_Y_ARB, n*sizeof(int), &(y[0]));
		glGetInternalformativ(Base::gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_Z_ARB, n*sizeof(int), &(z[0]));

		std::vector<glm::ivec3> sizes(n);
		for (int i = 0; i < n; ++i)
			sizes[i] = glm::ivec3{ x[i],y[i],z[i] };
		return sizes;
	}
};

class texture_sparse_2d : public texture_sparse_mipmapped<core_resource_type::Texture2D> {
private:
	using Base = texture_sparse_mipmapped<core_resource_type::Texture2D>;

public:
	texture_sparse_2d(gli::format format, const Base::size_type &size, int levels, const Base::size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_2d(texture_sparse_2d &&m) = default;
	texture_sparse_2d& operator=(texture_sparse_2d &&m) = default;

	static int max_size() {
		int s;
		glGetIntegerv(GL_MAX_SPARSE_TEXTURE_SIZE_ARB, &s);
		return s;
	}

	image<T> operator[](int level) const {
		return image<T>(*this, Base::get_image_container_size(), Base::format, image_access_mode::ReadWrite, level, 0);
	}
};

class texture_sparse_3d : public texture_sparse_mipmapped<core_resource_type::Texture3D> {
private:
	using Base = texture_sparse_mipmapped<core_resource_type::Texture3D>;

public:
	texture_sparse_3d(gli::format format, const Base::size_type &size, int levels, const Base::size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_3d(texture_sparse_3d &&m) = default;
	texture_sparse_3d& operator=(texture_sparse_3d &&m) = default;

	static int max_size() {
		int s;
		glGetIntegerv(GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB, &s);
		return s;
	}

	image_container<T> operator[](int level) const {
		return image_container<T>(*this, Base::get_image_container_size(), Base::format, image_access_mode::ReadWrite, level, Base::get_image_container_dimensions());
	}
};

class texture_sparse_2d_array : public texture_sparse_mipmapped<core_resource_type::Texture2DArray> {
private:
	using Base = texture_sparse_mipmapped<core_resource_type::Texture2DArray>;

public:
	texture_sparse_2d_array(gli::format format, const Base::size_type &size, int levels, const Base::size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_2d_array(texture_sparse_2d_array &&m) = default;
	texture_sparse_2d_array& operator=(texture_sparse_2d_array &&m) = default;

	static int max_layers() {
		int max_layers;
		glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB, &max_layers);
		return max_layers;
	}

	image_container<T> operator[](int level) const {
		return image_container<T>(*this, Base::get_image_container_size(), Base::format, image_access_mode::ReadWrite, level, Base::get_image_container_dimensions());
	}
};

}
}
