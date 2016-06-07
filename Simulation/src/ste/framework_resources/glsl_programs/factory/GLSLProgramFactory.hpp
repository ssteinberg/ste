// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "glsl_shader_object.hpp"
#include "glsl_program_object.hpp"

#include "program_binary.hpp"

#include "task_future.hpp"

#include <memory>

#include <string>
#include <list>
#include <map>

#include "StEngineControl.hpp"

#include "optional.hpp"

#include "boost_filesystem.hpp"

namespace StE {
namespace Resource {

class GLSLProgramFactory {
private:
	static const std::unordered_map<std::string, Core::GLSLShaderType> type_map;

private:
	~GLSLProgramFactory() {}

	static optional<boost::filesystem::path> resolve_program(const std::string &program_name);

	static std::string load_source(const boost::filesystem::path &path);

	static std::unique_ptr<Core::glsl_shader_object_generic> compile_from_path(const boost::filesystem::path &path);
	static std::unique_ptr<Core::glsl_shader_object_generic> compile_from_source(const boost::filesystem::path &path, std::string src, Core::GLSLShaderProperties prop, Core::GLSLShaderType);

	static std::vector<std::string> find_includes(const boost::filesystem::path &path);
	static bool parse_include(const boost::filesystem::path &, int, std::string &, std::vector<std::string> &);
	static bool parse_parameters(std::string &, Core::GLSLShaderProperties &, Core::GLSLShaderType &);
	static std::string parse_directive(const std::string &, const std::string &, std::string::size_type &, std::string::size_type &);

public:
	static task_future<std::unique_ptr<Core::glsl_program_object>> load_program_async(const StEngineControl &context, const std::vector<std::string> &names);
};

}
}
