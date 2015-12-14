
#include "stdafx.h"

#include "GLSLProgramLoader.h"
#include "program_binary.h"

#include "lru_cache.h"
#include "Log.h"

#include "AttributedString.h"
#include "attrib.h"

#include <exception>

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


const std::map<std::string, GLSLShaderType> GLSLProgramLoader::type_map = { { "compute", GLSLShaderType::COMPUTE },{ "frag", GLSLShaderType::FRAGMENT },{ "vert", GLSLShaderType::VERTEX },{ "geometry", GLSLShaderType::GEOMETRY },{ "tes", GLSLShaderType::TESS_EVALUATION },{ "tcs", GLSLShaderType::TESS_CONTROL } };

std::string GLSLProgramLoader::load_source(const boost::filesystem::path &path) {
	std::ifstream ifs(path.string(), std::ios::in);
	if (!ifs) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program";
		return std::string();
	}

	std::string code((std::istreambuf_iterator<char>(ifs)),
					 (std::istreambuf_iterator<char>()));

	return code;
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramLoader::compile_from_path(const boost::filesystem::path &path) {
	std::ifstream ifs(path.string(), std::ios::in);
	if (!ifs) {
		using namespace Attributes;
		ste_log_error() << AttributedString("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program.";
		return nullptr;
	}

	std::string line;
	std::string src;
	GLSLShaderProperties prop{ 0,0 };
	GLSLShaderType type = GLSLShaderType::NONE;

	std::vector<std::string> paths{ path.filename().string() };

	int i = 1;
	while (std::getline(ifs, line)) {
		if (line[0] == '#')
			parse_parameters(line, prop, type) || parse_include(path, i, line, paths);

		src += line + "\n";
		++i;
	}

	if (type == GLSLShaderType::NONE || prop.version_major == 0) {
		ste_log_error() << AttributedString("GLSL Shader ") + Attributes::i(path.string()) + ": No shader #type or #version specified.";
		return nullptr;
	}

	return compile_from_source(path, src, prop, type);
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramLoader::compile_from_source(const boost::filesystem::path &path, std::string code,
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

std::string GLSLProgramLoader::parse_directive(const std::string &source, const std::string &name, std::string::size_type &pos, std::string::size_type &end) {
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

bool GLSLProgramLoader::parse_parameters(std::string &line, GLSLShaderProperties &prop, GLSLShaderType &type) {
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

std::vector<boost::filesystem::path> GLSLProgramLoader::find_includes(const boost::filesystem::path &path) {
	std::vector<boost::filesystem::path> ret;
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

bool GLSLProgramLoader::parse_include(const boost::filesystem::path &path, int line, std::string &source, std::vector<std::string> &paths) {
	std::string::size_type it = 0, end;
	std::string name;
	std::string path_string = path.filename().string();

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

		auto include = load_source(file_name);
		source.insert(end, std::string("\n#line ") + std::to_string(line) + " \"" + path_string + "\"\n");
		source.replace(it, end - it, include);
		source.insert(it, std::string("#line 1 \"") + file_name + "\"\n");

		path_string = file_name;
		matched = true;
		paths.push_back(file_name);
	}

	return matched;
}

StE::task<std::unique_ptr<GLSLProgram>> GLSLProgramLoader::load_program_task(const StEngineControl &context, std::vector<boost::filesystem::path> files) {
	struct loader_data {
		program_binary bin;
		std::string cache_key;
		std::vector<boost::filesystem::path> files;
	};

	return StE::task<loader_data>([files = std::move(files), &context](optional<task_scheduler*> sched) -> loader_data {
		loader_data data;
		std::chrono::system_clock::time_point modification_time;

		{
			std::vector<boost::filesystem::path> paths = files;
			std::sort(paths.begin(), paths.end());
			data.cache_key = "glsl_program_binary_";

			for (unsigned i = 0; i < paths.size(); ++i) {
				auto includes = find_includes(paths[i]);
				for (auto &p : includes) {
					for (auto &s : paths) if (s == p) continue;
					paths.push_back(p);
				}
			}

			for (auto &path : paths) {
				if (!boost::filesystem::exists(path)) {
					ste_log_error() << "GLSL program " + path.string() + " couldn't be found!";
					assert(false);
					continue;
				}
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
		catch (std::exception *ex) {
			data.bin = program_binary();
		}

		data.files = std::move(files);

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
