
#include <stdafx.hpp>
#include <device_pipeline_shader_stage.hpp>
#include <ste_shader_spirv_parser.hpp>

using namespace ste::gl;

ste_shader_spirv_parser_output device_pipeline_shader_stage::verify_spirv_and_read_bindings(const std::string &code) {
	return ste_shader_spirv_parser::parse(code);
}
