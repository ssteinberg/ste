// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <memory>

#include "GLSLShader.h"

namespace StE {
namespace Resource {

class ShaderLoader {
private:
	~ShaderLoader() {}

public:
	static std::unique_ptr<LLR::GLSLShader> compile_from_path(const std::string & path);		// Auto type deduction based on extension
	static std::unique_ptr<LLR::GLSLShader> compile_from_path(const std::string & path, LLR::GLSLShader::GLSLShaderType type);
	static std::unique_ptr<LLR::GLSLShader> compile_source(const std::string & source, LLR::GLSLShader::GLSLShaderType type);
};

}
}
