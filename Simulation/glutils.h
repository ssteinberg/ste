// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace LLR {

class GLUtils {
public:
	GLUtils();

	static int query_gl_error(const char *, int);
	static void dump_gl_info(bool dumpExtensions = false);
};

}
}
