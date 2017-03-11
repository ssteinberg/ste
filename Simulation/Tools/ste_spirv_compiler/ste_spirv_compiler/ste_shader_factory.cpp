
#include "stdafx.h"

#include "ste_shader_factory.hpp"

#include <exception>

#include <type_traits>
#include <memory>

#include <fstream>
#include <sstream>
#include <locale>
#include <cctype>

#include <chrono>

#include <vector>
#include <algorithm>
#include <iostream>

#include <stdlib.h>

using namespace StE;

std::string ste_shader_factory::load_source(const boost::filesystem::path &path) {
	std::ifstream fs(path.string(), std::ios::in);
	if (!fs) {
		std::cerr << "Can not load " << path.string() << std::endl;
		throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
	}

	return std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
}

std::string ste_shader_factory::compile_from_path(const boost::filesystem::path &path,
												  const boost::filesystem::path &source_path,
												  shader_blob_header &header) {
	static const std::vector<std::string> inject_extenions = { "#extension GL_GOOGLE_cpp_style_line_directive : enable" };

	std::string line;
	std::string src;
	header.type = ste_shader_type::none;

	std::vector<std::string> paths{ path.filename().string() };

	std::ifstream fs(path.string(), std::ios::in);
	if (!fs) {
		std::cerr << "Can not load " << path.string() << std::endl;
		throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
	}

	for (int i = 1; std::getline(fs, line); ++i, src += line + "\n") {
		if (line[0] == '#') {
			if (parse_type(line, header))
				line = "";
			if (parse_parameters(line, header)) {
				line += "\n";
				for (auto &ext : inject_extenions)
					line += ext + "\n";
				line += std::string("#line ") + std::to_string(i) + " \"" + path.string() + "\"";
			}

			parse_include(path, i, line, paths, source_path);
		}
	}

	fs.close();

	if (header.type == ste_shader_type::none || header.properties.version_major == 0) {
		std::cerr << path.string() << ": Unknown type or version" << std::endl;
		throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
	}

	return src;
}

std::string ste_shader_factory::parse_directive(const std::string &source, const std::string &name, std::string::size_type &pos, std::string::size_type &end) {
	auto it = source.find(name, pos);
	pos = it;
	if (it == std::string::npos)
		return "";

	it += name.length();
	while (it < source.length() && std::isspace<char>(source[it], std::locale::classic())) ++it;
	end = source.find('\n', it);
	if (end == std::string::npos)
		end = source.length();

	return source.substr(it, end - it);
}

bool ste_shader_factory::parse_type(std::string &line, shader_blob_header &header) {
	std::string::size_type it = 0, end;

	std::string type_name = parse_directive(line, "#type", it, end);
	ste_shader_type t;
	if ((t = shader_type_from_type_string(type_name)) != ste_shader_type::none) {
		header.type = t;
		return true;
	}

	return false;
}

bool ste_shader_factory::parse_parameters(std::string &line, shader_blob_header &header) {
	std::string::size_type it = 0, end;

	std::string version = parse_directive(line, "#version", it, end);
	if (version.length() >= 3) {
		long lver = std::strtol(version.c_str(), nullptr, 10);
		header.properties.version_major = lver / 100;
		header.properties.version_minor = (lver - header.properties.version_major * 100) / 10;

		return true;
	}

	return false;
}

std::vector<std::string> ste_shader_factory::find_includes(const boost::filesystem::path &path) {
	std::vector<std::string> ret;
	std::string src = load_source(path);

	std::string::size_type it = 0, end;
	std::string name;
	while ((name = parse_directive(src, "#include", it, end)).length()) {
		if (name[0] != '<')
			break;
		auto name_len = name.find('>', 1);
		if (name_len == std::string::npos)
			break;

		std::string file_name = name.substr(1, name_len - 1);

		ret.push_back(file_name);

		it += sizeof("#include") + name_len + 1;
	}

	return ret;
}

bool ste_shader_factory::parse_include(const boost::filesystem::path &path, 
									   int line, 
									   std::string &source, 
									   std::vector<std::string> &paths,
									   const boost::filesystem::path &source_path) {
	std::string::size_type it = 0, end;
	std::string name;
	std::string path_string = path.string();

	if ((name = parse_directive(source, "#include", it, end)).length()) {
		if (name[0] != '<')
			return false;
		auto name_len = name.find('>', 1);
		if (name_len == std::string::npos)
			return false;

		std::string file_name = name.substr(1, name_len - 1);

		for (auto &p : paths)
			if (p == file_name) {
				source.replace(it, end - it, "");
				return false;
			}

		boost::filesystem::path include_path;
		bool has_path = resolve_program(file_name, &include_path, source_path);
		if (!has_path) {
			std::cerr << "Can not load include " << file_name << std::endl;
			throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
		}

		auto include = load_source(include_path);
		std::istringstream include_stream(include);
		std::string include_line, include_src;
		for (int i = 1; std::getline(include_stream, include_line); ++i, include_src += include_line + "\n") {
			if (include_line[0] == '#') parse_include(include_path, i, include_line, paths, source_path);
		}

		std::string include_path_string = include_path.string();
		std::replace(include_path_string.begin(), include_path_string.end(), '\\', '/');
		std::replace(path_string.begin(), path_string.end(), '\\', '/');

		include_src.insert(0, std::string("#line 1 \"") + include_path_string + "\"\n");
		source.insert(end, std::string("\n#line ") + std::to_string(line) + " \"" + path_string + "\"\n");
		source.replace(it, end - it, include_src);

		paths.push_back(file_name);

		return true;
	}

	return false;
}

bool ste_shader_factory::resolve_program(const std::string &program_name, 
										 boost::filesystem::path *path,
										 const boost::filesystem::path &source_path) {
	boost::filesystem::recursive_directory_iterator end;

	const auto it = std::find_if(boost::filesystem::recursive_directory_iterator(source_path),
								 end,
								 [&program_name](const boost::filesystem::directory_entry& e) {
		return e.path().filename() == program_name;
	});
	if (it == end)
		return false;
	
	*path = it->path();
	return true;
}

std::chrono::system_clock::time_point ste_shader_factory::shader_modification_time(const boost::filesystem::path &path, 
																				   const boost::filesystem::path &source_path) {
	std::chrono::system_clock::time_point modification_time;

	std::vector<boost::filesystem::path> paths = { path };

	for (unsigned i = 0; i < paths.size(); ++i) {
		auto &p = paths[i];

		auto includes = find_includes(p);
		for (auto &include : includes) {
			boost::filesystem::path i_path;
			if (!resolve_program(include, &i_path, source_path)) {
				std::cerr << "Can not load include " << include << std::endl;
				throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
			}

			if (std::find(paths.begin(), paths.end(), i_path) == paths.end())
				paths.push_back(i_path);
		}
	}

	for (auto &p : paths) {
		auto timet = boost::filesystem::last_write_time(p);
		std::chrono::system_clock::time_point sys_time_point = std::chrono::system_clock::from_time_t(timet);
		if (sys_time_point > modification_time) modification_time = sys_time_point;
	}

	return modification_time;
}

bool ste_shader_factory::compile_shader(const boost::filesystem::path &path,
										const boost::filesystem::path &source_path,
										const boost::filesystem::path &glslang_path,
										const boost::filesystem::path &shader_binary_output_path,
										const boost::filesystem::path &temp_path,
										shader_blob_header &out) {
	out = {};
	out.magic = shader_blob_header().magic;

	auto shader_name = path.stem();
	auto src = compile_from_path(path, source_path, out);
	std::string temp_extension;

	switch (out.type) {
	case ste_shader_type::compute_program:
		temp_extension = ".comp";
		break;
	case ste_shader_type::fragment_program:
		temp_extension = ".frag";
		break;
	case ste_shader_type::geometry_program:
		temp_extension = ".geom";
		break;
	case ste_shader_type::vertex_program:
		temp_extension = ".vert";
		break;
	case ste_shader_type::tesselation_control_program:
		temp_extension = ".tesc";
		break;
	case ste_shader_type::tesselation_evaluation_program:
		temp_extension = ".tese";
		break;
	default:
		std::cerr << "Unknown shader type" << std::endl;
		throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
	}

	auto temp_file_path = temp_path / (shader_name.string() + temp_extension);
	auto out_path = shader_binary_output_path;

	// Save temp file
	{
		std::ofstream temp_file(temp_file_path.string(), std::ofstream::out);
		if (!temp_file) {
			std::cerr << "Can not open temp file for writing: " << temp_file_path.string() << std::endl;
			throw std::exception((std::string(__FILE__) + ":" + std::to_string(__LINE__)).c_str());
		}
		temp_file << src;
	}

	// GLSL -> SPIR-v
	std::string cmd = glslang_path.string() + " -V -t -o \"" + 
		out_path.string() + "\" \"" + temp_file_path.string() + "\"";
	auto ret = system(cmd.c_str());

	boost::filesystem::remove(temp_file_path);

	return ret == 0;
}
