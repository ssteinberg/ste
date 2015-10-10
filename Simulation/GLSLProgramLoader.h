// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <memory>

#include <string>
#include <list>

#include "GLSLShader.h"
#include "GLSLProgram.h"

#include "task.h"
#include "StEngineControl.h"

namespace StE {
namespace Resource {

class GLSLProgramLoader {
private:
	~GLSLProgramLoader() {}

	static LLR::GLSLShader::GLSLShaderType type_from_path(const std::string &path) {
		if (path.length() < 4)
			return LLR::GLSLShader::GLSLShaderType::NONE;
		if (path.compare(path.length() - 4, 4, "vert") == 0)
			return LLR::GLSLShader::GLSLShaderType::VERTEX;
		if (path.compare(path.length() - 4, 4, "frag") == 0)
			return LLR::GLSLShader::GLSLShaderType::FRAGMENT;
		if (path.compare(path.length() - 4, 4, "glsl") == 0)
			return LLR::GLSLShader::GLSLShaderType::COMPUTE;
		if (path.compare(path.length() - 4, 4, "geom") == 0)
			return LLR::GLSLShader::GLSLShaderType::GEOMETRY;
		if (path.compare(path.length() - 4, 4, "tcs") == 0)
			return LLR::GLSLShader::GLSLShaderType::TESS_CONTROL;
		if (path.compare(path.length() - 4, 4, "tes") == 0)
			return LLR::GLSLShader::GLSLShaderType::TESS_EVALUATION;
		return LLR::GLSLShader::GLSLShaderType::NONE;
	}

	static std::string load_source(const std::string &path);

	static std::unique_ptr<LLR::GLSLShader> compile_from_path(const std::string & path);
	static std::unique_ptr<LLR::GLSLShader> compile_from_path(const std::string & path, LLR::GLSLShader::GLSLShaderType type);
	static std::unique_ptr<LLR::GLSLShader> compile_source(const std::string & source, LLR::GLSLShader::GLSLShaderType type);

public:
	static task<std::unique_ptr<LLR::GLSLProgram>> load_program_task(const StEngineControl &context, std::vector<std::string> files);
};

}
}
