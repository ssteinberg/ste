
#include "stdafx.h"
#include "gl_utils.h"

#include "Log.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdio>
#include <iostream>

using namespace StE::LLR;

gli::gl gl_utils::GL;

gl_utils::gl_utils() {}

int gl_utils::query_gl_error(std::string &out) {
	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		char buffer[256];
		_snprintf(buffer, 256, "%#08X", glErr);
		auto *str = gluErrorString(glErr);
		out = std::string(buffer) + ": \"" + (str ? reinterpret_cast<const char*>(str) : "Unknown GL Error") + "\"";
		return glErr;
	}
	return 0;
}

void gl_utils::dump_gl_info(bool dump_extensions) {
    const GLubyte *renderer = glGetString( GL_RENDERER );
    const GLubyte *vendor = glGetString( GL_VENDOR );
    const GLubyte *version = glGetString( GL_VERSION );
    const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

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
