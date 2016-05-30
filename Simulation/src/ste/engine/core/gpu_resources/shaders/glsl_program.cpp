
#include "stdafx.hpp"
#include "glsl_program.hpp"

#include "Log.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include <sys/stat.h>

using namespace StE::Core;

bool glsl_program::link() {
	if (linked) return true;
	if (!Base::is_valid()) return false;

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

		return false;
	}
	else {
		linked = true;
		return linked;
	}
}

bool glsl_program::link_from_binary(std::uint32_t format, const std::string &data) {
	glProgramBinary(get_resource_id(), format, data.data(), data.length());
	int success = 0;
	glGetProgramiv(get_resource_id(), GL_LINK_STATUS, &success);
	linked = success;
	return success;
}

std::string glsl_program::get_binary_represantation(std::uint32_t *format) {
	int bin_len = 0;
	glGetProgramiv(get_resource_id(), GL_PROGRAM_BINARY_LENGTH, &bin_len);
	std::string data;
	data.resize(bin_len);
	glGetProgramBinary(get_resource_id(), bin_len, NULL, format, &data[0]);

	return data;
}
