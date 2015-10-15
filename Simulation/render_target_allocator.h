// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "resource_allocator.h"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class RenderTargetAllocator : public generic_resource_immutable_storage_allocator<int, const gli::gl::format &, const glm::tvec2<std::size_t> &> {
public:
	unsigned allocate() override final {
		GLuint id;
		glCreateRenderbuffers(1, &id);
		return id;
	}
	void deallocate(unsigned &id) override final {
		if (is_valid(id))
			glDeleteRenderbuffers(1, &id);
		id = 0;
	}

	void allocate_storage(unsigned id, int samples, const gli::gl::format &format, const glm::tvec2<std::size_t> &size) {
		glNamedRenderbufferStorageMultisample(id, samples, format.Internal, size.x, size.y);
	}
};

}
}
