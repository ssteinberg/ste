// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <memory>

#include <string>
#include <list>
#include <map>

#include "GLSLShader.h"
#include "GLSLProgram.h"

#include "task.h"
#include "StEngineControl.h"

#include "optional.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

namespace StE {
namespace Resource {

class GLSLProgramLoader {
private:
	static const std::map<std::string, LLR::GLSLShaderType> type_map;

private:
	~GLSLProgramLoader() {}

	static std::string load_source(const boost::filesystem::path &path);

	static std::unique_ptr<LLR::GLSLShaderGeneric> compile_from_path(const boost::filesystem::path &path);
	static std::unique_ptr<LLR::GLSLShaderGeneric> compile_from_source(const boost::filesystem::path &path, std::string src);

	static std::vector<boost::filesystem::path> find_includes(const boost::filesystem::path &path);
	static void parse_includes(const boost::filesystem::path &path, std::string &);
	static bool parse_parameters(const boost::filesystem::path &path, std::string &, LLR::GLSLShaderProperties &, LLR::GLSLShaderType &);
	static std::string parse_directive(const std::string &, const std::string &, std::string::size_type &, std::string::size_type &);

public:
	static task<std::unique_ptr<LLR::GLSLProgram>> load_program_task(const StEngineControl &context, std::vector<boost::filesystem::path> files);
};

}
}
