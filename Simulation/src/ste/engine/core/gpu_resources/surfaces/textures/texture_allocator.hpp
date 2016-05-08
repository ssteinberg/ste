// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "resource_allocator.hpp"
#include "texture_traits.hpp"

namespace StE {
namespace Core {

namespace _texture_storage {

template <int dim, bool ms> void inline create_gl_texture_storage(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<dim>::type &size) {
	assert(false && "Unspecialized");
}
template <> void inline create_gl_texture_storage<1, false>(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<1>::type &size) {
	glTextureStorage1D(id, levels, format.Internal, size);
}
template <> void inline create_gl_texture_storage<2, false>(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
	glTextureStorage2D(id, levels, format.Internal, size[0], size[1]);
}
template <> void inline create_gl_texture_storage<3, false>(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
	glTextureStorage3D(id, levels, format.Internal, size[0], size[1], size[2]);
}
template <> void inline create_gl_texture_storage<2, true>(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<2>::type &size) {
	glTextureStorage2DMultisample(id, samples, format.Internal, size[0], size[1], false);
}
template <> void inline create_gl_texture_storage<3, true>(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<3>::type &size) {
	glTextureStorage3DMultisample(id, samples, format.Internal, size[0], size[1], size[2], false);
}

}

template <core_resource_type tex_type>
class texture_immutable_storage_allocator : public generic_resource_immutable_storage_allocator<int, int, const gli::gl::format &, const typename texture_size_type<texture_dimensions<tex_type>::dimensions>::type &, std::size_t> {
	using Base = generic_resource_immutable_storage_allocator<int, int, const gli::gl::format &, const typename texture_size_type<texture_dimensions<tex_type>::dimensions>::type &, std::size_t>;

private:
	static constexpr int dimensions = texture_dimensions<tex_type>::dimensions;
	static constexpr bool multisampled = texture_is_multisampled<tex_type>::value;

public:
	GenericResource::type allocate() override final {
		GLuint id;
		glCreateTextures(GL::gl_utils::translate_type(tex_type), 1, &id);
		return id;
	}

	static void deallocate(GenericResource::type &id) {
		if (id) {
			glDeleteTextures(1, &id);
			id = 0;
		}
	}

	void allocate_storage(GenericResource::type id, int levels, int samples, const gli::gl::format &format, const typename texture_size_type<dimensions>::type &size, std::size_t bytes) override final {
		_texture_storage::create_gl_texture_storage<dimensions, multisampled>(id, levels, samples, format, size);
	}
};

}
}
