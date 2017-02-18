
#include <stdafx.hpp>

#include <glsl_program_factory.hpp>
#include <glsl_program_factory_exceptions.hpp>

#include <lru_cache.hpp>
#include <Log.hpp>

#include <attributed_string.hpp>
#include <attrib.hpp>

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
using StE::Core::glsl_shader_object_generic;
using StE::Core::glsl_shader_object;
using StE::Core::glsl_shader_type;
using StE::Core::glsl_shader_properties;
using StE::Core::glsl_program_object;


const std::unordered_map<std::string, glsl_shader_type> glsl_program_factory::type_map = { { "compute", glsl_shader_type::COMPUTE },{ "frag", glsl_shader_type::FRAGMENT },{ "vert", glsl_shader_type::VERTEX },{ "geometry", glsl_shader_type::GEOMETRY },{ "tes", glsl_shader_type::TESS_EVALUATION },{ "tcs", glsl_shader_type::TESS_CONTROL } };

std::string glsl_program_factory::load_source(const boost::filesystem::path &path) {
	std::ifstream fs(path.string(), std::ios::in);
	if (!fs) {
		using namespace Attributes;
		ste_log_error() << attributed_string("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program - " + std::strerror(errno) << std::endl;
		throw resource_io_error();
	}

	return std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
}

std::unique_ptr<glsl_shader_object_generic> glsl_program_factory::compile_from_path(const boost::filesystem::path &path) {
	std::string line;
	std::string src;
	glsl_shader_properties prop{ 0,0 };
	glsl_shader_type type = glsl_shader_type::NONE;

	std::vector<std::string> paths{ path.filename().string() };

	std::ifstream fs(path.string(), std::ios::in);
	if (!fs) {
		using namespace Attributes;
		ste_log_error() << attributed_string("GLSL Shader ") + i(path.string()) + ": Unable to read GLSL shader program - " + std::strerror(errno) << std::endl;
		throw resource_io_error();
	}

	for (int i = 1; std::getline(fs, line); ++i, src += line + "\n") {
		if (line[0] == '#')
			parse_parameters(line, prop, type) || parse_include(path, i, line, paths);
	}

	fs.close();

	if (type == glsl_shader_type::NONE || prop.version_major == 0) {
		using namespace Attributes;
		ste_log_error() << attributed_string("GLSL Shader ") + i(path.string()) + ": No shader #type or #version specified." << std::endl;
		throw glsl_program_undefined_program_error();
	}

	return compile_from_source(path, src, prop, type);
}

std::unique_ptr<glsl_shader_object_generic> glsl_program_factory::compile_from_source(const boost::filesystem::path &path, std::string code,
																					glsl_shader_properties prop, glsl_shader_type type) {
#ifdef _DEBUG
	{
		std::ofstream otemp(std::string("tmp/") + path.filename().string() + ".tmp");
		otemp << code << std::endl;
	}
#endif

	std::string name = path.filename().string();

	std::unique_ptr<glsl_shader_object_generic> shader;
	switch (type) {
	case glsl_shader_type::VERTEX:			shader = std::make_unique<glsl_shader_object<glsl_shader_type::VERTEX>>(name, code, prop); break;
	case glsl_shader_type::FRAGMENT:			shader = std::make_unique<glsl_shader_object<glsl_shader_type::FRAGMENT>>(name, code, prop); break;
	case glsl_shader_type::GEOMETRY:			shader = std::make_unique<glsl_shader_object<glsl_shader_type::GEOMETRY>>(name, code, prop); break;
	case glsl_shader_type::COMPUTE:			shader = std::make_unique<glsl_shader_object<glsl_shader_type::COMPUTE>>(name, code, prop); break;
	case glsl_shader_type::TESS_CONTROL:		shader = std::make_unique<glsl_shader_object<glsl_shader_type::TESS_CONTROL>>(name, code, prop); break;
	case glsl_shader_type::TESS_EVALUATION:	shader = std::make_unique<glsl_shader_object<glsl_shader_type::TESS_EVALUATION>>(name, code, prop); break;
	default:
		throw glsl_program_undefined_shader_error();
	}

	if (!shader->is_valid()) {
		using namespace Attributes;
		ste_log_error() << attributed_string("GLSL Shader ") + i(name) + ": Unable to create GLSL shader program!" << std::endl;

		throw glsl_program_shader_compilation_error();
	}

	if (!shader->get_status()) {
		using namespace Attributes;
		ste_log_error() << attributed_string("GLSL Shader ") + i(name) + ": Compiling GLSL shader failed! Reason: " << shader->read_info_log() << std::endl;

		throw glsl_program_shader_compilation_error();
	}

	ste_log() << "Successfully compiled GLSL shader \"" << Attributes::i(name) << "\"." << std::endl;

	return std::move(shader);
}

std::string glsl_program_factory::parse_directive(const std::string &source, const std::string &name, std::string::size_type &pos, std::string::size_type &end) {
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

bool glsl_program_factory::parse_parameters(std::string &line, glsl_shader_properties &prop, glsl_shader_type &type) {
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

std::vector<std::string> glsl_program_factory::find_includes(const boost::filesystem::path &path) {
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

bool glsl_program_factory::parse_include(const boost::filesystem::path &path, int line, std::string &source, std::vector<std::string> &paths) {
	std::string::size_type it = 0, end;
	std::string name;
	std::string path_string = path.string();

	if ((name = parse_directive(source, "#include", it, end)).length()) {
		if (name[0] != '"')
			return false;
		auto name_len = name.find('"', 1);
		if (name_len == std::string::npos)
			return false;

		std::string file_name = name.substr(1, name_len - 1);

		for (auto &p : paths)
			if (p == file_name) {
				source.replace(it, end - it, "");
				return false;
			}

		auto include_path = resolve_program(file_name);
		if (!include_path) {
			ste_log_error() << "GLSL program " + file_name + " couldn't be found!" << std::endl;
			throw resource_io_error();
		}

		auto include = load_source(*include_path);
		std::istringstream include_stream(include);
		std::string include_line, include_src;
		for (int i = 1; std::getline(include_stream, include_line); ++i, include_src += include_line + "\n") {
			if (include_line[0] == '#') parse_include(*include_path, i, include_line, paths);
		}

		std::string include_path_string = include_path->string();
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

StE::optional<boost::filesystem::path> glsl_program_factory::resolve_program(const std::string &program_name) {
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

StE::task_future<std::unique_ptr<glsl_program_object>> glsl_program_factory::load_program_async(const ste_engine_control &context, const std::vector<std::string> &names) {
	struct loader_data {
		program_binary bin;
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
					ste_log_error() << "GLSL program " + program_name + " couldn't be found!" << std::endl;
					throw resource_io_error();
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
						ste_log_error() << "GLSL program " + p + " couldn't be found!" << std::endl;
						throw resource_io_error();
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
			auto cache_get_lambda = context.cache().get<program_binary>(data.cache_key);
			optional<program_binary> opt = cache_get_lambda();
			if (opt && opt->get_time_point() > modification_time)
				data.bin = opt.get();
		}
		catch (const std::exception &) {
			data.bin = program_binary();
		}

		return data;
		// TODO: Fix
	}).then/*_on_main_thread*/([=, &context](loader_data data) -> std::unique_ptr<glsl_program_object> {
		if (data.bin.blob.length()) {
			std::unique_ptr<glsl_program_object> program = std::make_unique<glsl_program_object>();
			if (program->link_from_binary(data.bin.format, data.bin.blob)) {
				ste_log() << "Successfully linked GLSL program from cached binary" << std::endl;
				return program;
			}
		}

		std::unique_ptr<glsl_program_object> program = std::make_unique<glsl_program_object>();
		for (auto &shader_path : data.files)
			program->add_shader(compile_from_path(shader_path));

		auto prog_name = program->generate_name();
		ste_log() << "Linking GLSL program \"" << Text::Attributes::i(prog_name) << "\".";
		if (!program->link())
			throw glsl_program_linking_error();

		ste_log() << "Successfully linked GLSL program \"" << Text::Attributes::i(prog_name) << "\" from source." << std::endl;

		data.bin.blob = program->get_binary_representation(&data.bin.format);
		data.bin.set_time_point(std::chrono::system_clock::now());

		context.cache().insert(data.cache_key, std::move(data.bin));

		return program;
	});
}
