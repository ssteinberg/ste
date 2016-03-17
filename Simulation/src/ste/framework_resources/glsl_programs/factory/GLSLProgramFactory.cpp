
#include "stdafx.hpp"

#include "GLSLProgramFactory.hpp"
#include "program_binary.hpp"

#include "lru_cache.hpp"
#include "Log.hpp"

#include "AttributedString.hpp"
#include "attrib.hpp"

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

using namespace StE::Resource;
using namespace StE::Resource::glsl_loader;
using namespace StE::Text;
using StE::LLR::GLSLShaderGeneric;
using StE::LLR::GLSLShader;
using StE::LLR::GLSLShaderType;
using StE::LLR::GLSLShaderProperties;
using StE::LLR::GLSLProgram;


const std::map<std::string, GLSLShaderType> GLSLProgramFactory::type_map = { { "compute", GLSLShaderType::COMPUTE },{ "frag", GLSLShaderType::FRAGMENT },{ "vert", GLSLShaderType::VERTEX },{ "geometry", GLSLShaderType::GEOMETRY },{ "tes", GLSLShaderType::TESS_EVALUATION },{ "tcs", GLSLShaderType::TESS_CONTROL } };

std::string GLSLProgramFactory::load_source(const boost::filesystem::path &path) {
	std::ifstream fs(path.string(), std::ios::in);
	if (!fs.good()) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program - " + std::strerror(errno) << std::endl;
		return std::string();
	}
	
	return std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramFactory::compile_from_path(const boost::filesystem::path &path) {
	std::string line;
	std::string src;
	GLSLShaderProperties prop{ 0,0 };
	GLSLShaderType type = GLSLShaderType::NONE;

	std::vector<std::string> paths{ path.filename().string() };
	
	std::ifstream fs(path.string(), std::ios::in);
	if (!fs.good()) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program - " + std::strerror(errno) << std::endl;
		return nullptr;
	}

	for (int i = 1; std::getline(fs, line); ++i, src += line + "\n") {
		if (line[0] == '#')
			parse_parameters(line, prop, type) || parse_include(path, i, line, paths);
	}
	
	fs.close();

	if (type == GLSLShaderType::NONE || prop.version_major == 0) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": No shader #type or #version specified.";
		return nullptr;
	}

	return compile_from_source(path, src, prop, type);
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramFactory::compile_from_source(const boost::filesystem::path &path, std::string code,
																		  GLSLShaderProperties prop, GLSLShaderType type) {
	std::unique_ptr<GLSLShaderGeneric> shader;
	switch (type) {
	case GLSLShaderType::VERTEX:	shader = std::make_unique<GLSLShader<GLSLShaderType::VERTEX>>(code, prop); break;
	case GLSLShaderType::FRAGMENT:	shader = std::make_unique<GLSLShader<GLSLShaderType::FRAGMENT>>(code, prop); break;
	case GLSLShaderType::GEOMETRY:	shader = std::make_unique<GLSLShader<GLSLShaderType::GEOMETRY>>(code, prop); break;
	case GLSLShaderType::COMPUTE:	shader = std::make_unique<GLSLShader<GLSLShaderType::COMPUTE>>(code, prop); break;
	case GLSLShaderType::TESS_CONTROL: shader = std::make_unique<GLSLShader<GLSLShaderType::TESS_CONTROL>>(code, prop); break;
	case GLSLShaderType::TESS_EVALUATION: shader = std::make_unique<GLSLShader<GLSLShaderType::TESS_EVALUATION>>(code, prop); break;
	}

	if (!shader->is_valid()) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Unable to create GLSL shader program!";
		return false;
	}

	if (!shader->get_status()) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Compiling GLSL shader failed! Reason: " << shader->read_info_log();

		return nullptr;
	}

	ste_log() << "Successfully compiled GLSL shader";

	return std::move(shader);
}

std::string GLSLProgramFactory::parse_directive(const std::string &source, const std::string &name, std::string::size_type &pos, std::string::size_type &end) {
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

bool GLSLProgramFactory::parse_parameters(std::string &line, GLSLShaderProperties &prop, GLSLShaderType &type) {
	std::string::size_type it = 0, end;

	std::string version = parse_directive(line, "#version", it, end);
	if (version.length() >= 3) {
		long lver = std::strtol(version.c_str(), nullptr, 10);
		prop.version_major = lver / 100;
		prop.version_minor = (lver - prop.version_major * 100) / 10;

		return true;
	}

	it = 0;
	auto mapit = type_map.find(parse_directive(line, "#type", it, end));
	if (mapit != type_map.end()) {
		type = mapit->second;

		line = "";

		return true;
	}

	return false;
}

std::vector<std::string> GLSLProgramFactory::find_includes(const boost::filesystem::path &path) {
	std::vector<std::string> ret;
	std::string src = load_source(path);

	std::string::size_type it = 0, end;
	std::string name;
	while ((name = parse_directive(src, "#include", it, end)).length()) {
		if (name[0] != '"')
			break;
		auto name_len = name.find('"', 1);
		if (name_len == std::string::npos)
			break;

		std::string file_name = name.substr(1, name_len - 1);

		ret.push_back(file_name);

		it += sizeof("#include") + name_len + 1;
	}

	return ret;
}

bool GLSLProgramFactory::parse_include(const boost::filesystem::path &path, int line, std::string &source, std::vector<std::string> &paths) {
	std::string::size_type it = 0, end;
	std::string name;
	std::string path_string = path.string();

	bool matched = false;

	while ((name = parse_directive(source, "#include", it, end)).length()) {
		if (name[0] != '"')
			break;
		auto name_len = name.find('"', 1);
		if (name_len == std::string::npos)
			break;

		std::string file_name = name.substr(1, name_len - 1);

		bool duplicate = false;
		for (auto &p : paths)
			if (p == file_name) {
				source.replace(it, end - it, "");
				duplicate = true;
				break;
			}
		if (duplicate)
			continue;

		if (matched) {
			line = 0;
			for (unsigned i = 0; i < it; ++i) if (source[i] == '\n') ++line;
		}
		
		auto include_path = resolve_program(file_name);
		if (!include_path) {
			ste_log_error() << "GLSL program " + file_name + " couldn't be found!";
			std::cerr << "Error: GLSL program " + file_name + " couldn't be found";
			assert(false);
			return false;
		}

		auto include = load_source(*include_path);
		source.insert(end, std::string("\n#line ") + std::to_string(line) + " \"" + path_string + "\"\n");
		source.replace(it, end - it, include);
		source.insert(it, std::string("#line 1 \"") + include_path->string() + "\"\n");

		path_string = include_path->string();
		matched = true;
		paths.push_back(file_name);
	}

	return matched;
}

StE::optional<boost::filesystem::path> GLSLProgramFactory::resolve_program(const std::string &program_name) {
	boost::filesystem::recursive_directory_iterator end;
	
	const auto it = std::find_if(boost::filesystem::recursive_directory_iterator("."),
								 end,
								 [&program_name](const boost::filesystem::directory_entry& e) {
									 return e.path().filename() == program_name;
								 });
	if (it == end)
		return none;
	return it->path();
}

StE::task<std::unique_ptr<GLSLProgram>> GLSLProgramFactory::load_program_task(const StEngineControl &context, const std::vector<std::string> &names) {
	struct loader_data {
		program_binary bin;
		std::string cache_key;
		std::vector<boost::filesystem::path> files;
	};

	return StE::task<loader_data>([names = std::move(names), &context](optional<task_scheduler*> sched) -> loader_data {
		loader_data data;
		std::chrono::system_clock::time_point modification_time;

		{
			std::vector<boost::filesystem::path> paths;
			for (auto &program_name : names) {
				auto path = resolve_program(program_name);
				if (!path) {
					ste_log_error() << "GLSL program " + program_name + " couldn't be found!";
					std::cerr << "Error: GLSL program " + program_name + " couldn't be found";
					assert(false);
					continue;
				}
				
				paths.push_back(*path);
			}
			
			std::sort(paths.begin(), paths.end());
			
			data.files = paths;
			data.cache_key = "glsl_program_binary_";

			for (unsigned i = 0; i < paths.size(); ++i) {
				auto includes = find_includes(paths[i]);
				for (auto &p : includes) {
					auto path = resolve_program(p);
					if (!path) {
						ste_log_error() << "GLSL program " + p + " couldn't be found!";
						std::cerr << "Error: GLSL program " + p + " couldn't be found";
						assert(false);
						continue;
					}
				
					for (auto &s : paths) if (s == *path) continue;
					paths.push_back(*path);
				}
			}

			for (auto &path : paths) {
				auto timet = boost::filesystem::last_write_time(path);
				std::chrono::system_clock::time_point sys_time_point = std::chrono::system_clock::from_time_t(timet);
				if (sys_time_point > modification_time) modification_time = sys_time_point;
				data.cache_key += path.string() + "_";
			}
		}

		try {
			auto cache_get_task = context.cache().get<program_binary>(data.cache_key);
			optional<program_binary> opt = cache_get_task();
			if (opt && opt->get_time_point() > modification_time)
				data.bin = opt.get();
		}
		catch (const std::exception &ex) {
			data.bin = program_binary();
		}

		return data;
	}).then_on_main_thread([=, &context](optional<task_scheduler*> sched, loader_data data) -> std::unique_ptr<LLR::GLSLProgram> {
		if (data.bin.blob.length()) {
			std::unique_ptr<GLSLProgram> program = std::make_unique<GLSLProgram>();
			if (program->link_from_binary(data.bin.format, data.bin.blob)) {
				ste_log() << "Successfully linked GLSL program from cached binary";
				return program;
			}
		}

		std::unique_ptr<GLSLProgram> program = std::make_unique<GLSLProgram>();
		for (auto &shader_path : data.files)
			program->add_shader(compile_from_path(shader_path));
		if (!program->link())
			return nullptr;

		data.bin.blob = program->get_binary_represantation(&data.bin.format);
		data.bin.set_time_point(std::chrono::system_clock::now());

		context.cache().insert(data.cache_key, std::move(data.bin));

		return program;
	});
}
