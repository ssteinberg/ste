// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable_layout_validator.hpp>

#include <pipeline_layout_attachment_location.hpp>

namespace StE {
namespace GL {

struct ste_shader_stage_attachment : ste_shader_stage_variable_layout_validator {
	pipeline_layout_attachment_location location;
};

}
}
