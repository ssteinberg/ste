// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <glsl_program_factory.hpp>

#include <resource_instance.hpp>
#include <resource_loading_task.hpp>
#include <resource_instance_getter.hpp>

#include <glsl_program_object.hpp>

#include <memory>

namespace StE {
namespace Resource {

class glsl_program {
	friend class resource_loading_task<glsl_program>;
	friend class resource_instance_getter<glsl_program>;
	friend class resource_instance<glsl_program>;

private:
	std::vector<std::string> names;
	std::unique_ptr<Core::glsl_program_object> program;

private:
	glsl_program(const ste_engine_control &ctx, const std::vector<std::string> &names) : names(names) {}
	glsl_program(const ste_engine_control &ctx, const std::string &name) : names({ name }) {}
};

template <>
class resource_loading_task<glsl_program> {
	using R = glsl_program;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return glsl_program_factory::load_program_async(ctx, object->names)
		// TODO: Fix
				.then/*_on_main_thread*/([object](std::unique_ptr<Core::glsl_program_object> &&program) {
					object->program = std::move(program);
					object->names.clear();
				});
	}
};

template <>
class resource_instance_getter<glsl_program> {
	using R = glsl_program;

public:
	auto &get(R *res) {
		return *res->program;
	}
	const auto &get(const R *res) {
		return *res->program;
	}
};

}
}
