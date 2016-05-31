// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "glsl_program.hpp"
#include "Scene.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_prepopulate_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;

	Resource::resource_instance<Resource::glsl_program> program;

public:
	scene_prepopulate_depth_dispatch(const StEngineControl &ctx, const Scene *scene) : scene(scene),
																					   program(ctx, std::vector<std::string>{ "scene_prepopulate_depth.vert", "scene_prepopulate_depth.frag" }) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
