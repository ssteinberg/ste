// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage_attachment.hpp>

#include <vector>

namespace StE {
namespace GL {

struct ste_shader_spirv_parser_output {
	std::vector<ste_shader_stage_binding> bindings;
	std::vector<ste_shader_stage_attachment> attachments;
};

}
}
