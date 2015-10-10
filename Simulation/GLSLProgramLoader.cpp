
#include "stdafx.h"

#include "GLSLProgramLoader.h"
#include "program_binary.h"

#include "lru_cache.h"
#include "Log.h"

#include <exception>

#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>

#include <boost/crc.hpp>

using namespace StE::Resource;
using namespace StE::Resource::glsl_loader;
using StE::LLR::GLSLShader;
using StE::LLR::GLSLProgram;

std::unique_ptr<GLSLShader> GLSLProgramLoader::compile_from_path(const std::string & path) {
	auto type = type_from_path(path);
	if (type == LLR::GLSLShader::GLSLShaderType::NONE)
		return nullptr;
	return compile_from_path(path, type);
}

std::string GLSLProgramLoader::load_source(const std::string &path) {
	std::ifstream inFile(path, std::ios::in);
	if (!inFile) {
		ste_log_error() << "Unable to read GLSL shader program: " << path;
		return std::string();
	}

	std::ostringstream code;
	while (inFile.good()) {
		int c = inFile.get();
		if (!inFile.eof()) code << (char)c;
	}
	inFile.close();

	return code.str();
}

std::unique_ptr<GLSLShader> GLSLProgramLoader::compile_from_path(const std::string &path, GLSLShader::GLSLShaderType type) {
	auto code = load_source(path);
	if (!code.length())
		return nullptr;
	return compile_source(code, type);
}

std::unique_ptr<GLSLShader> GLSLProgramLoader::compile_source(const std::string &source, GLSLShader::GLSLShaderType type) {
	std::unique_ptr<GLSLShader> shader(new GLSLShader(type));
	if (!shader->is_valid()) {
		ste_log_error() << "Unable to create GLSL shader program!";
		return false;
	}

	shader->set_shader_source(source);

	if (!shader->compile()) {
		// Compile failed, log and return false
		ste_log_error() << "Compiling GLSL shader failed! Reason: " << shader->read_info_log();

		return nullptr;
	}

	ste_log() << "Successfully compiled GLSL shader";

	return std::move(shader);
}

StE::task<std::unique_ptr<GLSLProgram>> GLSLProgramLoader::load_program_task(const StEngineControl &context, std::vector<std::string> files) {
	struct loader_data {
		std::vector<std::pair<std::string, LLR::GLSLShader::GLSLShaderType>> sources;
		program_binary bin;
		std::string cache_key;
		std::uint64_t crc;
	};

	return StE::task<loader_data>([files = std::move(files), &context](optional<task_scheduler*> sched) -> loader_data {
		loader_data data;

		decltype(files) paths = files;
		std::sort(paths.begin(), paths.end());
		data.cache_key = "glsl_program_binary_";

		for (auto &path : paths) {
			auto type = type_from_path(path);
			if (type == LLR::GLSLShader::GLSLShaderType::NONE)
				return loader_data();

			auto code = load_source(path);
			if (!code.length())
				return loader_data();

			data.sources.push_back(std::make_pair(code, type));
			data.cache_key += path;
		}

		std::uint64_t crc = 0;
		for (auto &str : data.sources) {
			boost::crc_32_type crc_ccitt;
			crc_ccitt = std::for_each(str.first.data(), str.first.data() + str.first.length(), crc_ccitt);
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
		for (auto &shader_source : data.sources)
			program->add_shader(compile_source(shader_source.first, shader_source.second));
		if (!program->link())
			return nullptr;

		data.bin.blob = program->get_binary_represantation(&data.bin.format);
		data.bin.crc = data.crc;

		context.cache().insert(data.cache_key, std::move(data.bin));

		return program;
	});
}
