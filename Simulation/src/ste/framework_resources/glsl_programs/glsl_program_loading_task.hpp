// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "GLSLProgramFactory.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "glsl_program.hpp"

#include <memory>

namespace StE {
namespace Resource {

template <>
class resource_loading_task<Core::glsl_program> {
	using R = Core::glsl_program;

public:
	auto loader(const StEngineControl &ctx, const std::vector<std::string> &names) {
		return GLSLProgramFactory::load_program_task(ctx, names);
	}
	auto loader(const StEngineControl &ctx, const std::string &name) {
		return GLSLProgramFactory::load_program_task(ctx, { name });
	}
};

}
}
