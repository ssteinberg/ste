
#include "stdafx.h"

#include "GLSLProgramLoader.h"
#include "program_binary.h"

#include "lru_cache.h"
#include "Log.h"

#include <exception>

#include <fstream>
#include <sstream>
#include <locale>
#include <cctype>

#include <vector>
#include <algorithm>

#include <boost/crc.hpp>

using namespace StE::Resource;
using namespace StE::Resource::glsl_loader;
using StE::LLR::GLSLShaderGeneric;
using StE::LLR::GLSLShader;
using StE::LLR::GLSLShaderType;
using StE::LLR::GLSLShaderProperties;
using StE::LLR::GLSLProgram;


const std::map<std::string, GLSLShaderType> GLSLProgramLoader::type_map = { { "compute", GLSLShaderType::COMPUTE },{ "frag", GLSLShaderType::FRAGMENT },{ "vert", GLSLShaderType::VERTEX },{ "geometry", GLSLShaderType::GEOMETRY },{ "tes", GLSLShaderType::TESS_EVALUATION },{ "tcs", GLSLShaderType::TESS_CONTROL } };

std::string GLSLProgramLoader::load_source(const std::string &path) {
	std::ifstream ifs(path, std::ios::in);
	if (!ifs) {
		ste_log_error() << "Unable to read GLSL shader program: " << path;
		return std::string();
	}

	std::string code((std::istreambuf_iterator<char>(ifs)),
					 (std::istreambuf_iterator<char>()));

	return code;
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramLoader::compile_from_path(const std::string &path) {
	auto code = load_source(path);
	if (!code.length())
		return nullptr;
	return compile_from_source(code);
}

std::unique_ptr<GLSLShaderGeneric> GLSLProgramLoader::compile_from_source(std::string code) {
	parse_includes(code);

	GLSLShaderProperties prop;
	GLSLShaderType type;
	if (!parse_parameters(code, prop, type)) {
		return false;
	}

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
		ste_log_error() << "Unable to create GLSL shader program!";
		return false;
	}

	if (!shader->get_status()) {
		// Compile failed, log and return false
		ste_log_error() << "Compiling GLSL shader failed! Reason: " << shader->read_info_log();

		return nullptr;
	}

	ste_log() << "Successfully compiled GLSL shader";

	return std::move(shader);
}

std::string GLSLProgramLoader::parse_directive(const std::string &source, const std::string &name, std::string::size_type &pos, std::string::size_type &end) {
	auto it = source.find(name);
	pos = it;
	if (it == std::string::npos)
		return "";

	it += name.length();
	while (it < source.length() && std::isspace<char>(source[it], std::locale::classic()));
	end = source.find('\n', it);
	if (end == std::string::npos)
		return "";

	return source.substr(it, end - it);
}

bool GLSLProgramLoader::parse_parameters(std::string & source, GLSLShaderProperties &prop, GLSLShaderType &type) {
	std::string::size_type it = 0, end;

	std::string version = parse_directive(source, "#version", it, end);
	if (version.length() < 3) {
		ste_log_error() << "GLSL Shader: malformed #version directive";
		assert(false);
		return false;
	}
	long lver = std::strtol(version.c_str(), nullptr, 10);
	prop.version_major = lver / 100;
	prop.version_minor = (lver - prop.version_major) / 10;

	it = 0;
	auto mapit = type_map.find(parse_directive(source, "#type", it, end));
	if (mapit == type_map.end()) {
		ste_log_error() << "GLSL Shader: malformed #type directive or unknown type";
		assert(false);
		return false;
	}
	type = mapit->second;

	return true;
}

void GLSLProgramLoader::parse_includes(std::string &source) {
	std::string::size_type it = 0, end;
	std::string name;
	while ((name = parse_directive(source, "#include", it, end)).length()) {
		if (name[0] != '"')
			break;
		auto end = name.find('"', 1);
		if (end == std::string::npos)
			break;

		std::string file_name = name.substr(1, end - 1);
		++end;

		auto include = load_source(file_name);
		source.replace(it, end - it, include);
	}

	if (it != std::string::npos) {
		ste_log_error() << "GLSL Shader: malformed #include directive";
		assert(false);
	}
}

StE::task<std::unique_ptr<GLSLProgram>> GLSLProgramLoader::load_program_task(const StEngineControl &context, std::vector<std::string> files) {
	struct loader_data {
		std::vector<std::string> sources;
		program_binary bin;
		std::string cache_key;
		std::uint64_t crc;
	};

	return StE::task<loader_data>([files = std::move(files), &context](optional<task_scheduler*> sched) -> loader_data {
		loader_data data;

		{
			decltype(files) paths = files;
			std::sort(paths.begin(), paths.end());
			data.cache_key = "glsl_program_binary_";

			for (auto &path : paths) {
				auto code = load_source(path);
				if (!code.length())
					return loader_data();
				parse_includes(code);

				data.sources.push_back(code);
				data.cache_key += path;
			}
		}

		std::uint64_t crc = 0;
		for (auto &str : data.sources) {
			boost::crc_32_type crc_ccitt;
			crc_ccitt = std::for_each(str.data(), str.data() + str.length(), crc_ccitt);
			crc += crc_ccitt();
		}

		data.crc = crc;

		try {
			auto cache_get_task = context.cache().get<program_binary>(data.cache_key);
			optional<program_binary> opt = cache_get_task();
			if (opt && opt->crc == crc)
				data.bin = opt.get();
		}
		catch (std::exception *ex) {
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
		for (auto &shader_src : data.sources)
			program->add_shader(compile_from_source(shader_src));
		if (!program->link())
			return nullptr;

		data.bin.blob = program->get_binary_represantation(&data.bin.format);
		data.bin.crc = data.crc;

		context.cache().insert(data.cache_key, std::move(data.bin));

		return program;
	});
}
