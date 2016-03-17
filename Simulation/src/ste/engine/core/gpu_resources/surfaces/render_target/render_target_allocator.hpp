// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "resource_allocator.hpp"

#include <gli/gli.hpp>

namespace StE {
namespace Core {

class RenderTargetAllocator : public generic_resource_immutable_storage_allocator<int, const gli::gl::format &, const glm::ivec2 &, std::size_t> {
	using Base = generic_resource_immutable_storage_allocator<int, const gli::gl::format &, const glm::ivec2 &, std::size_t>;
	
public:
	GenericResource::type allocate() override final {
		GLuint id;
		glCreateRenderbuffers(1, &id);
		return id;
	}
	static void deallocate(GenericResource::type &id) {
		if (id) {
			glDeleteRenderbuffers(1, &id);
			id = 0;
		}
	}

	void allocate_storage(GenericResource::type id, int samples, const gli::gl::format &format, const glm::ivec2 &size, std::size_t bytes) {
		glNamedRenderbufferStorageMultisample(id, samples, format.Internal, size.x, size.y);
	}
};

}
}
