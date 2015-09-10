
#include "stdafx.h"
#include "GLSLProgram.h"

#include "Log.h"

#include <fstream>
using std::ifstream;
using std::ios;
using std::string;

#include <iostream>
#include <sstream>
using std::ostringstream;

#include <sys/stat.h>

using namespace StE::LLR;

bool GLSLProgram::link() {
	if (linked) return true;
	if (!is_valid()) return false;
	
	for (auto &shader : shaders)
		shader->attach_to_program(id);

	ste_log() << "Linking GLSL program";

	glLinkProgram(id);

	int status = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &status);
	if (!status) {
		// Log and return false
		int length = 0;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

		std::string reason;
		if (length > 0) {
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(id, length, &written, c_log);
			reason = c_log;
			delete[] c_log;
		}

		ste_log_error() << "Linking GLSL program failed! Reason: " << reason;

		return false;
	}
	else {
		linked = true;
		return linked;
	}
}

void GLSLProgram::set_uniform(const char *name, float x, float y, float z) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform3f(loc, x, y, z);
}

void GLSLProgram::set_uniform(const char *name, const glm::vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2f(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const char *name, const glm::i32vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2i(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const char *name, const glm::vec3 & v) const {
	this->set_uniform(name, v.x, v.y, v.z);
}

void GLSLProgram::set_uniform(const char *name, const glm::vec4 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void GLSLProgram::set_uniform(const char *name, const glm::mat2 & m) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniformMatrix2fv(loc, 1, GL_FALSE, &m[0][0]);
}

void GLSLProgram::set_uniform(const char *name, const glm::mat3 & m) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void GLSLProgram::set_uniform(const char *name, const glm::mat4 & m) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void GLSLProgram::set_uniform(const char *name, float val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform1f(loc, val);
}

void GLSLProgram::set_uniform(const char *name, int val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform1i(loc, val);
}

void GLSLProgram::set_uniform(const char *name, bool val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform1i(loc, val);
}

void GLSLProgram::print_active_uniforms() {
	GLint nUniforms, size, location, maxLen;
	GLchar * name;
	GLsizei written;
	GLenum type;

	glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &nUniforms);

	name = (GLchar *)malloc(maxLen);

	printf(" Location | Name\n");
	printf("------------------------------------------------\n");
	for (int i = 0; i < nUniforms; ++i) {
		glGetActiveUniform(id, i, maxLen, &written, &size, &type, name);
		location = glGetUniformLocation(id, name);
		printf(" %-8d | %s\n", location, name);
	}

	free(name);
}

void GLSLProgram::print_active_attribs() {
	GLint written, size, location, maxLength, nAttribs;
	GLenum type;
	GLchar * name;

	glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
	glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &nAttribs);

	name = (GLchar *)malloc(maxLength);

	printf(" Index | Name\n");
	printf("------------------------------------------------\n");
	for (int i = 0; i < nAttribs; i++) {
		glGetActiveAttrib(id, i, maxLength, &written, &size, &type, name);
		location = glGetAttribLocation(id, name);
		printf(" %-5d | %s\n", location, name);
	}

	free(name);
}

int GLSLProgram::get_uniform_location(const char * name) const {
	return glGetUniformLocation(id, name);
}
