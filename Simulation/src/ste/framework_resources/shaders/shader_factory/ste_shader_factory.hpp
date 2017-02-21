// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_shader_type.hpp>
#include <ste_shader_properties.hpp>

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include <optional.hpp>
#include <boost_filesystem.hpp>

namespace StE {
namespace Resource {

class ste_shader_factory {
private:
	~ste_shader_factory() noexcept {}

	static bool resolve_program(const std::string &program_name, boost::filesystem::path *path, const boost::filesystem::path &source_path);

	static std::string load_source(const boost::filesystem::path &path);
	static std::string compile_from_path(const boost::filesystem::path &path, const boost::filesystem::path &source_path, ste_shader_type *type);

	static std::vector<std::string> find_includes(const boost::filesystem::path &path);
	static bool parse_include(const boost::filesystem::path &, int, std::string &, std::vector<std::string> &, const boost::filesystem::path &);
	static bool parse_type(std::string &, ste_shader_type &);
	static bool parse_parameters(std::string &, ste_shader_properties &);
	static std::string parse_directive(const std::string &, const std::string &, std::string::size_type &, std::string::size_type &);

public:
	static std::chrono::system_clock::time_point shader_modification_time(const boost::filesystem::path &path, const boost::filesystem::path &source_path);
	static bool compile_shader(const boost::filesystem::path &path,
							   const boost::filesystem::path &source_path,
							   const boost::filesystem::path &glslang_path,
							   const boost::filesystem::path &shader_binary_output_path,
							   const boost::filesystem::path &temp_path);
};

}
}
