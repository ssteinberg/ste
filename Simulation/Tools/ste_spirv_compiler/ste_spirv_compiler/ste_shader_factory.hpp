// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.h"

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

namespace StE {

struct ste_shader_properties {
	std::uint32_t version_major, version_minor;
};

enum class ste_shader_type : std::uint32_t {
	vertex_program = 0,
	tesselation_control_program = 1,
	tesselation_evaluation_program = 2,
	geometry_program = 3,
	fragment_program = 4,
	compute_program = 5,

	none = 0xFFFFFFFF,
};

auto inline shader_type_from_type_string(const std::string &type) {
	if (type == "vert")
		return ste_shader_type::vertex_program;
	if (type == "frag")
		return ste_shader_type::fragment_program;
	if (type == "geometry")
		return ste_shader_type::geometry_program;
	if (type == "tes")
		return ste_shader_type::tesselation_evaluation_program;
	if (type == "tcs")
		return ste_shader_type::tesselation_control_program;
	if (type == "compute")
		return ste_shader_type::compute_program;

	return ste_shader_type::none;
}

struct shader_blob_header {
	std::uint32_t magic{ 0x058ADE20 };
	ste_shader_type type;
	ste_shader_properties properties;
};

class ste_shader_factory {
private:
	~ste_shader_factory() noexcept {}

	static bool resolve_program(const std::string &program_name, boost::filesystem::path *path, const boost::filesystem::path &source_path);

	static std::string load_source(const boost::filesystem::path &path);
	static std::string compile_from_path(const boost::filesystem::path &path,
										 const boost::filesystem::path &source_path,
										 shader_blob_header &);

	static std::vector<std::string> find_includes(const boost::filesystem::path &path);
	static bool parse_include(const boost::filesystem::path &, int, std::string &, std::vector<std::string> &, const boost::filesystem::path &);
	static bool parse_type(std::string &, shader_blob_header &);
	static bool parse_parameters(std::string &, shader_blob_header &);
	static std::string parse_directive(const std::string &, const std::string &, std::string::size_type &, std::string::size_type &);

public:
	static std::chrono::system_clock::time_point shader_modification_time(const boost::filesystem::path &path, const boost::filesystem::path &source_path);
	static std::string compile_shader(const boost::filesystem::path &path,
									  const boost::filesystem::path &source_path,
									  shader_blob_header &out);
};

}
