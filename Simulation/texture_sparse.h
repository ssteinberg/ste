// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "texture.h"
#include "llr_resource_type.h"

#include <vector>

namespace StE {
namespace LLR {

template <llr_resource_type type>
class texture_sparse_mipmapped : public texture_mipmapped<type> {
private:
	using Base = texture_mipmapped<type>;

	void upload_level(const void *data, int level = 0, int layer = 0, LLRCubeMapFace face = LLRCubeMapFace::LLRCubeMapFaceNone, int data_size = 0) override {}

protected:
	size_type tile_size;
	int max_sparse_level;

	texture_sparse_mipmapped(gli::format format, const size_type &size, int levels, const size_type &tile_size, int virtual_page_idx) : Base(), tile_size(tile_size) {
		this->levels = levels;
		allocate_tex_storage(size, format, levels, 1, true, virtual_page_idx);
		glGetTextureParameteriv(get_resource_id(), GL_NUM_SPARSE_LEVELS_ARB, &max_sparse_level);
	}

public:
	texture_sparse_mipmapped(texture_sparse_mipmapped &&m) = default;
	texture_sparse_mipmapped& operator=(texture_sparse_mipmapped &&m) = default;

	auto get_tile_size() const { return tile_size; }
	auto get_max_sparse_level() const { return max_sparse_level; }

	void commit_tiles(const size_type &offset_size, const size_type &count_size, int level) {
		glm::tvec3<std::size_t> offset3{ 0,0,0 };
		glm::tvec3<std::size_t> count3{ 1,1,1 };
		for (int i = 0; i < dimensions(); ++i) {
			offset3[i] = offset_size[i];
			count3[i] = count_size[i];
		}

		glTexturePageCommitmentEXT(get_resource_id(), level, offset3.x, offset3.y, offset3.z, count3.x, count3.y, count3.z, true);
	}
	void uncommit_tiles(const size_type &offset_size, const size_type &count_size, int level) {
		glm::tvec3<std::size_t> offset3{ 0,0,0 };
		glm::tvec3<std::size_t> count3{ 1,1,1 };
		for (int i = 0; i < dimensions(); ++i) {
			offset3[i] = offset_size[i];
			count3[i] = count_size[i];
		}

		glTexturePageCommitmentEXT(get_resource_id(), level, offset3.x, offset3.y, offset3.z, count3.x, count3.y, count3.z, false);
	}

	static std::vector<glm::ivec3> page_sizes(gli::format gli_format) {
		gli::gl::format const format = gl_utils::translate_format(gli_format);

		int n;
		glGetInternalformativ(gl_type(), format.External, GL_NUM_VIRTUAL_PAGE_SIZES_ARB, 1, &n);
		std::vector<int> x(n), y(n), z(n);
		glGetInternalformativ(gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_X_ARB, n*sizeof(int), &(x[0]));
		glGetInternalformativ(gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_Y_ARB, n*sizeof(int), &(y[0]));
		glGetInternalformativ(gl_type(), format.External, GL_VIRTUAL_PAGE_SIZE_Z_ARB, n*sizeof(int), &(z[0]));

		std::vector<glm::ivec3> sizes(n);
		for (int i = 0; i < n; ++i)
			sizes[i] = glm::ivec3{ x[i],y[i],z[i] };
		return sizes;
	}
};

class texture_sparse_2d : public texture_sparse_mipmapped<llr_resource_type::LLRTexture2D> {
private:
	using Base = texture_sparse_mipmapped<T>;

public:
	texture_sparse_2d(gli::format format, const size_type &size, int levels, const size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_2d(texture_sparse_2d &&m) = default;
	texture_sparse_2d& operator=(texture_sparse_2d &&m) = default;

	static int max_size() {
		int s;
		glGetIntegerv(GL_MAX_SPARSE_TEXTURE_SIZE_ARB, &s);
		return s;
	}
};

class texture_sparse_3d : public texture_sparse_mipmapped<llr_resource_type::LLRTexture3D> {
private:
	using Base = texture_sparse_mipmapped<T>;

public:
	texture_sparse_3d(gli::format format, const size_type &size, int levels, const size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_3d(texture_sparse_3d &&m) = default;
	texture_sparse_3d& operator=(texture_sparse_3d &&m) = default;

	static int max_size() {
		int s;
		glGetIntegerv(GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB, &s);
		return s;
	}
};

class texture_sparse_2d_array : public texture_sparse_mipmapped<llr_resource_type::LLRTexture2DArray> {
private:
	using Base = texture_sparse_mipmapped<T>;

public:
	texture_sparse_2d_array(gli::format format, const size_type &size, int levels, const size_type &tile_size, int virtual_page_idx) : Base(format, size, levels, tile_size, virtual_page_idx) {}

	texture_sparse_2d_array(texture_sparse_2d_array &&m) = default;
	texture_sparse_2d_array& operator=(texture_sparse_2d_array &&m) = default;

	static int max_layers() {
		int max_layers;
		glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB, &max_layers);
		return max_layers;
	}
};

}
}
