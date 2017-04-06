
#include <stdafx.hpp>
#include <device_pipeline_shader_stage.hpp>
#include <ste_shader_spirv_bindings_parser.hpp>

using namespace StE::GL;

std::vector<ste_shader_stage_binding> device_pipeline_shader_stage::verify_spirv_and_read_bindings(const std::string &code) {
	return ste_shader_spirv_bindings_parser::parse_bindings(code);
}
