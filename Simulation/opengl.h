// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <gli/gli.hpp>

#include "llr_resource.h"

namespace StE {
namespace LLR {

class opengl {
private:
	static gli::gl GL;

public:
	opengl();

	static int query_gl_error(const char *, int);
	static void dump_gl_info(bool dump_extensions = false);

	static gli::gl::format gl_translate_format(const gli::format &format) { return GL.translate(format); }
	static constexpr GLenum gl_translate_type(const llr_resource_type &type) { return static_cast<GLenum>(type); }
};

}
}
