// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "resource.hpp"

#include <gli/gli.hpp>
#include <string>
#include <thread>

namespace StE {
namespace Core {

class gl_utils {
private:
	static gli::gl GL;

public:
	gl_utils();

	static int query_gl_error(std::string &out);
	static void dump_gl_info(bool dump_extensions = false);

	static gli::gl::format translate_format(const gli::format &format, const gli::swizzles &swizzle) { return GL.translate(format, swizzle); }
	static constexpr GLenum translate_type(const core_resource_type &type) { return static_cast<GLenum>(type); }
};

}
}
