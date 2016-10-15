// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"

#include "glsl_program.hpp"
#include "Scene.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_prepopulate_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;
	bool front_face;

	Resource::resource_instance<Resource::glsl_program> program;

public:
	scene_prepopulate_depth_dispatch(const StEngineControl &ctx, 
									 const Scene *scene,
									 bool front_face = true) : scene(scene),
															   front_face(front_face),
															   program(ctx, std::vector<std::string>{ "scene_transform.vert", "scene_prepopulate_depth.frag" }) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
