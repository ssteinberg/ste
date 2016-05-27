// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "GLSLShader.hpp"
#include "GLSLProgram.hpp"

#include "program_binary.hpp"

#include <memory>

#include <string>
#include <list>
#include <map>

#include "StEngineControl.hpp"

#include "optional.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

namespace StE {
namespace Resource {

class GLSLProgramFactory {
private:
	static const std::unordered_map<std::string, Core::GLSLShaderType> type_map;

private:
	~GLSLProgramFactory() {}

	static optional<boost::filesystem::path> resolve_program(const std::string &program_name);

	static std::string load_source(const boost::filesystem::path &path);

	static std::unique_ptr<Core::GLSLShaderGeneric> compile_from_path(const boost::filesystem::path &path);
	static std::unique_ptr<Core::GLSLShaderGeneric> compile_from_source(const boost::filesystem::path &path, std::string src, Core::GLSLShaderProperties prop, Core::GLSLShaderType);

	static std::vector<std::string> find_includes(const boost::filesystem::path &path);
	static bool parse_include(const boost::filesystem::path &, int, std::string &, std::vector<std::string> &);
	static bool parse_parameters(std::string &, Core::GLSLShaderProperties &, Core::GLSLShaderType &);
	static std::string parse_directive(const std::string &, const std::string &, std::string::size_type &, std::string::size_type &);

public:
	static auto load_program_task(const StEngineControl &context, const std::vector<std::string> &names) {
		struct loader_data {
			glsl_loader::program_binary bin;
			std::string cache_key;
			std::vector<boost::filesystem::path> files;
		};

		return context.scheduler().schedule_now([names = std::move(names), &context]() -> loader_data {
			loader_data data;
			std::chrono::system_clock::time_point modification_time;

			{
				std::vector<boost::filesystem::path> paths;
				for (auto &program_name : names) {
					auto path = resolve_program(program_name);
					if (!path) {
						ste_log_error() << "GLSL program " + program_name + " couldn't be found!";
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
				auto cache_get_task = context.cache().get<glsl_loader::program_binary>(data.cache_key);
				optional<glsl_loader::program_binary> opt = cache_get_task();
				if (opt && opt->get_time_point() > modification_time)
					data.bin = opt.get();
			}
			catch (const std::exception &ex) {
				data.bin = glsl_loader::program_binary();
			}

			return data;
		}).then_on_main_thread([=, &context](loader_data data) -> std::unique_ptr<Core::GLSLProgram> {
			if (data.bin.blob.length()) {
				std::unique_ptr<Core::GLSLProgram> program = std::make_unique<Core::GLSLProgram>();
				if (program->link_from_binary(data.bin.format, data.bin.blob)) {
					ste_log() << "Successfully linked GLSL program from cached binary";
					return program;
				}
			}

			std::unique_ptr<Core::GLSLProgram> program = std::make_unique<Core::GLSLProgram>();
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
};

}
}
