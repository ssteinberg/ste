// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "resource_allocator.h"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class RenderTargetAllocator : public generic_resource_immutable_storage_allocator<int, const gli::gl::format &, const glm::ivec2 &, std::size_t> {
	using Base = generic_resource_immutable_storage_allocator<int, const gli::gl::format &, const glm::ivec2 &, std::size_t>;
	
protected:
	unsigned allocate() override final {
		GLuint id;
		glCreateRenderbuffers(1, &id);
		return id;
	}
	static void deallocate(unsigned &id) {
		if (is_valid(id))
			glDeleteRenderbuffers(1, &id);
		id = 0;
	}

public:
	void allocate_storage(unsigned id, int samples, const gli::gl::format &format, const glm::ivec2 &size, std::size_t bytes) {
		glNamedRenderbufferStorageMultisample(id, samples, format.Internal, size.x, size.y);
	}
};

}
}
