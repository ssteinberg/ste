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
	boost::filesystem::path path = R"(D:\src\git\StE\Simulation\src\ste\framework_graphics\antialiasing\fxaa\shaders\fxaa.frag)";
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

		auto spirv_temp_output = temp_path / (std::string("compiled_blob_") + path.filename().string());
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

		StE::shader_blob_header header;
		if (!StE::ste_shader_factory::compile_shader(path,
													 source_path,
													 glslang_path,
													 spirv_temp_output,
													 temp_path,
													 header)) {
			throw std::exception("Compile failed");
		}


		std::string blob;
		{
			std::ifstream fs;
			fs.exceptions(fs.exceptions() | std::ios::failbit | std::ifstream::badbit);
			fs.open(spirv_temp_output.string(), std::ios::in | std::ios::binary);
			blob = std::string((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		}
		boost::filesystem::remove(spirv_temp_output);

		{
			std::ofstream of;
			of.exceptions(of.exceptions() | std::ios::failbit | std::ifstream::badbit);
			of.open(output.string(), std::ios::out | std::ios::binary);
			of.write(reinterpret_cast<const char*>(&header), sizeof(header));
			of.write(blob.data(), blob.size());
		}
	}
	catch (std::exception e) {
		return 1;
	}

	return 0;
}
