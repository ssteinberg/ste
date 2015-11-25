
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
		glProgramUniform2f(get_resource_id(), loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::i32vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2i(get_resource_id(), loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::u32vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2ui(get_resource_id(), loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::i64vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2i64ARB(get_resource_id(), loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::u64vec2 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2ui64ARB(get_resource_id(), loc, v.x, v.y);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::vec3 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform3f(get_resource_id(), loc, v.x, v.y, v.z);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::vec4 & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glProgramUniform4f(get_resource_id(), loc, v.x, v.y, v.z, v.w);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat2 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformMatrix2fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat3 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformMatrix3fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, const glm::mat4 & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glProgramUniformMatrix4fv(get_resource_id(), loc, 1, transpose, &m[0][0]);
}

void GLSLProgram::set_uniform(const std::string &name, float val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glProgramUniform1f(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::int32_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1i(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::int64_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1i64ARB(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::uint32_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1ui(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, std::uint64_t val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1ui64ARB(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, bool val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0) 
		glProgramUniform1i(get_resource_id(), loc, val);
}

void GLSLProgram::set_uniform(const std::string &name, const texture_handle &handle) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformHandleui64ARB(get_resource_id(), loc, handle);
}

void GLSLProgram::set_uniform_subroutine(GLenum shader_type, const std::vector<unsigned> &ids) const {
	glUniformSubroutinesuiv(shader_type, ids.size(), &ids[0]);
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::vec2> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2fv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::i32vec2> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2iv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::u32vec2> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2uiv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::i64vec2> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2i64vARB(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::u64vec2> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform2ui64vARB(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::vec3> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform3fv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::vec4> & v) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform4fv(get_resource_id(), loc, v.size(), reinterpret_cast<const std::remove_reference_t<decltype(v)>::value_type::value_type*>(&v[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::mat2> & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformMatrix2fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::mat3> & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformMatrix3fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<glm::mat4> & m, bool transpose) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformMatrix4fv(get_resource_id(), loc, m.size(), transpose, reinterpret_cast<const float*>(&m[0][0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<float> & val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1fv(get_resource_id(), loc, val.size(), reinterpret_cast<const std::remove_reference_t<decltype(val)>::value_type*>(&val[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<std::int32_t> &val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1iv(get_resource_id(), loc, val.size(), reinterpret_cast<const std::remove_reference_t<decltype(val)>::value_type*>(&val[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<std::int64_t> &val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1i64vARB(get_resource_id(), loc, val.size(), reinterpret_cast<const std::remove_reference_t<decltype(val)>::value_type*>(&val[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<std::uint32_t> &val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1uiv(get_resource_id(), loc, val.size(), reinterpret_cast<const std::remove_reference_t<decltype(val)>::value_type*>(&val[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<std::uint64_t> &val) const {
	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniform1ui64vARB(get_resource_id(), loc, val.size(), reinterpret_cast<const std::remove_reference_t<decltype(val)>::value_type*>(&val[0]));
}

void GLSLProgram::set_uniform(const std::string &name, const std::vector<std::reference_wrapper<const texture_handle>> &handles) const {
	std::vector<decltype(texture_handle().get_handle())> v;
	for (auto &ref : handles)
		v.push_back(ref.get().get_handle());

	int loc = get_uniform_location(name);
	if (loc >= 0)
		glProgramUniformHandleui64vARB(get_resource_id(), loc, v.size(), &v[0]);
}
