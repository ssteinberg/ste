// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "llr_resource.h"

#include <gli/gli.hpp>

namespace StE {
namespace LLR {

class RenderTargetAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateRenderbuffers(1, &id); return id; }
	static void deallocate(GLuint &id) { glDeleteRenderbuffers(1, &id); id = 0; }
};

class RenderTarget : public llr_resource<RenderTargetAllocator> {
protected:
	glm::tvec2<std::size_t> size;
	gli::format format;

public:
	RenderTarget(RenderTarget &&m) = default;
	RenderTarget& operator=(RenderTarget &&m) = default;

	RenderTarget(gli::format format, const glm::tvec2<std::size_t> &size, int samples = 0) : size(size), format(format) {
		glNamedRenderbufferStorageMultisample(id, samples, opengl::gl_translate_format(format).Internal, size.x, size.y);
	}

	glm::tvec2<std::size_t> get_size() const { return size; }
	gli::format get_format() const { return format; }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRRenderBufferObject; }
};

}
}
