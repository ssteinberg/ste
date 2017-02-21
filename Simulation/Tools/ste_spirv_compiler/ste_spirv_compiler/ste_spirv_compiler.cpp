// ste_spirv_compiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ste_shader_factory.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include <iostream>

int main(int argc,
		 char *argv[],
		 char *envp[]) {
	boost::filesystem::path path = R"(D:\src\git\StE\Simulation\src\ste\framework_graphics\renderers\global_illumination\deferred_composer\shaders\deferred_compose.frag)";
	boost::filesystem::path source_path = R"(D:\src\git\StE\Simulation\src)";
	boost::filesystem::path glslang_path = R"(D:\src\git\glslang\glslang\install\bin\glslangValidator.exe)";
	boost::filesystem::path shader_binary_output_path = R"(D:\src\git\StE\Simulation\Data\programs)";
	boost::filesystem::path temp_path = R"(D:\src\git\StE\Simulation\temp)";

#ifndef _DEBUG
	if (argc != 2) {
		std::cerr << "Path not specified" << std::endl;
		return 2;
	}

	path = argv[1];
#endif

	try {
		if (!boost::filesystem::exists(path)) {
			std::cerr << "Path doesn't exist" << std::endl;
			return 2;
		}

		auto output = shader_binary_output_path / path.filename();

//#ifndef _DEBUG
		if (boost::filesystem::exists(output)) {
			std::chrono::system_clock::time_point target_modification_time =
				std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(output));
			auto source_modification_time = StE::ste_shader_factory::shader_modification_time(path, source_path);
			if (source_modification_time <= target_modification_time)
				return 0;
		}
//#endif

		if (!StE::ste_shader_factory::compile_shader(path,
													 source_path,
													 glslang_path,
													 output,
													 temp_path)) {
			throw std::exception("Compile failed");
		}
	}
	catch (std::exception e) {
		return 1;
	}

	return 0;
}
