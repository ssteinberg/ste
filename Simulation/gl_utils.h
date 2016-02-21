// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "resource.h"

#include <gli/gli.hpp>
#include <string>
#include <thread>

namespace StE {
namespace LLR {

class gl_utils {
private:
	static gli::gl GL;

public:
	gl_utils();

	static int query_gl_error(std::string &out);
	static void dump_gl_info(bool dump_extensions = false);

	static gli::gl::format translate_format(const gli::format &format) { return GL.translate(format, gli::swizzles(gli::SWIZZLE_ZERO)); }
	static constexpr GLenum translate_type(const llr_resource_type &type) { return static_cast<GLenum>(type); }
};

}
}
