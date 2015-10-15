
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
		glAttachShader(get_resource_id(), shader->get_resource_id());

	ste_log() << "Linking GLSL program";

	glLinkProgram(get_resource_id());

	shaders.clear();

	int status = 0;
	glGetProgramiv(get_resource_id(), GL_LINK_STATUS, &status);
	if (!status) {
		// Log and return false
		int length = 0;
		glGetProgramiv(get_resource_id(), GL_INFO_LOG_LENGTH, &length);

		std::string reason;
		if (length > 0) {
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(get_resource_id(), length, &written, c_log);
			reason = c_log;
			delete[] c_log;
		}

		ste_log_error() << "Linking GLSL program failed! Reason: " << reason;
		assert(false);

		return false;
	}
	else {
		linked = true;
		return linked;
	}
}

bool GLSLProgram::link_from_binary(unsigned format, const std::string &data) {
	glProgramBinary(get_resource_id(), format, data.data(), data.length());
	int success = 0;
	glGetProgramiv(get_resource_id(), GL_LINK_STATUS, &success);
	linked = success;
	return success;
}

std::string GLSLProgram::get_binary_represantation(unsigned *format) {
	int bin_len = 0;
	glGetProgramiv(get_resource_id(), GL_PROGRAM_BINARY_LENGTH, &bin_len);
	std::string data;
	data.resize(bin_len);
	glGetProgramBinary(get_resource_id(), bin_len, NULL, format, &data[0]);

	return data;
}

void GLSLProgram::set_uniform(const std::string &name, const glm::vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2f(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::i32vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2i(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::u32vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2ui(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::i64vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2i64ARB(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::u64vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform2ui64ARB(loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::vec3 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform3f(loc, v.x, v.y, v.z);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::vec4 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat2 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniformMatrix2fv(loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat3 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniformMatrix3fv(loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat4 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniformMatrix4fv(loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, float val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform1f(loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::int32_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform1i(loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::int64_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform1i64ARB(loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::uint32_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform1ui(loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::uint64_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glUniform1ui64ARB(loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, bool val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glUniform1i(loc, val);
}
