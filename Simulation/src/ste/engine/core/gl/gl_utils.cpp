
#include "stdafx.hpp"
#include "gl_utils.hpp"

#include "Log.hpp"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdio>
#include <iostream>

using namespace StE::Core::GL;

gli::gl gl_utils::GL(gli::gl::profile::PROFILE_GL33);

gl_utils::gl_utils() {}

int gl_utils::query_gl_error(std::string &out) {
	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		char buffer[256];
		snprintf(buffer, 256, "%#08X", glErr);
		auto *str = gluErrorString(glErr);
		out = std::string(buffer) + ": \"" + (str ? reinterpret_cast<const char*>(str) : "Unknown GL Error") + "\"";
		return glErr;
	}
	return 0;
}

std::string gl_utils::get_vendor() {
	const GLubyte *vendor = glGetString(GL_VENDOR);
	return std::string(reinterpret_cast<const char*>(vendor));
}

std::string gl_utils::get_renderer() {
	const GLubyte *renderer = glGetString(GL_RENDERER);
	return std::string(reinterpret_cast<const char*>(renderer));
}

std::string gl_utils::get_gl_version() {
	const GLubyte *version = glGetString(GL_VERSION);
	return std::string(reinterpret_cast<const char*>(version));
}

std::string gl_utils::get_glsl_version() {
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	return std::string(reinterpret_cast<const char*>(glslVersion));
}

void gl_utils::dump_gl_info(bool dump_extensions) {
	const std::string renderer = get_renderer();
	const std::string vendor = get_vendor();
	const std::string version = get_gl_version();
	const std::string glslVersion = get_glsl_version();

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	ste_log() << "GL Vendor\t: " << vendor << std::endl;
	ste_log() << "GL Renderer\t: " << renderer << std::endl;
	ste_log() << "GL Version\t: " << version << std::endl;
	ste_log() << "GL Version\t: " << major << "," << minor << std::endl;
	ste_log() << "GLSL Version\t: " << glslVersion << std::endl;

	if(dump_extensions) {
		GLint nExtensions;
		glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
		for( int i = 0; i < nExtensions; i++ )
			ste_log() << glGetStringi(GL_EXTENSIONS, i) << std::endl;
	}
}
