#include "stdafx.h"

#include "ShaderLoader.h"
#include "Log.h"

#include <fstream>
#include <sstream>

using namespace StE::Resource;
using StE::LLR::GLSLShader;

std::unique_ptr<GLSLShader> ShaderLoader::compile_from_path(const std::string & path) {
	if (path.length() < 4)
		return nullptr;
	if (path.compare(path.length() - 4, 4, "vert") == 0)
		return compile_from_path(path, GLSLShader::VERTEX);
	if (path.compare(path.length() - 4, 4, "frag") == 0)
		return compile_from_path(path, GLSLShader::FRAGMENT);
	if (path.compare(path.length() - 4, 4, "glsl") == 0)
		return compile_from_path(path, GLSLShader::COMPUTE);
	return nullptr;
}

std::unique_ptr<GLSLShader> ShaderLoader::compile_from_path(const std::string &path, GLSLShader::GLSLShaderType type) {
	std::ifstream inFile(path, std::ios::in);
	if (!inFile) {
		ste_log_error() << "Unable to read GLSL shader program: " << path;
		return nullptr;
	}

	std::ostringstream code;
	while (inFile.good()) {
		int c = inFile.get();
		if (!inFile.eof()) code << (char)c;
	}
	inFile.close();

	ste_log() << "Compiling GLSL shader " << path;

	return compile_source(code.str(), type);
}

std::unique_ptr<GLSLShader> ShaderLoader::compile_source(const std::string &source, GLSLShader::GLSLShaderType type) {
	std::unique_ptr<GLSLShader> shader(new GLSLShader(type));
	if (!shader->is_valid()) {
		ste_log_error() << "Unable to create GLSL shader program!";
		return false;
	}

	shader->set_shader_source(source);

	if (!shader->compile()) {
		// Compile failed, log and return false
		ste_log_error() << "Compiling GLSL shader failed! Reason: " << shader->read_info_log();

		return nullptr;
	}

	ste_log() << "Successfully compiled GLSL shader";

	return std::move(shader);
}
