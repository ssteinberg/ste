// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "ste_engine_control.hpp"

#include "glsl_program.hpp"
#include "scene.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_prepopulate_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const scene *s;
	bool front_face;

	Resource::resource_instance<Resource::glsl_program> program;

public:
	scene_prepopulate_depth_dispatch(const ste_engine_control &ctx, 
									 const scene *s,
									 bool front_face = true) : s(s),
															   front_face(front_face),
															   program(ctx, std::vector<std::string>{ "scene_transform.vert", "scene_prepopulate_depth.frag" }) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
