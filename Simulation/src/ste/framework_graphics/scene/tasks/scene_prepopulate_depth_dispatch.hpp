// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"
#include "ObjectGroup.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene;

class scene_prepopulate_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;

	std::shared_ptr<Core::GLSLProgram> program;

public:
	scene_prepopulate_depth_dispatch(const StEngineControl &ctx,
									 const Scene *scene) : scene(scene),
														   program(ctx.glslprograms_pool().fetch_program_task({ "scene_prepopulate_depth.vert", "scene_prepopulate_depth.frag" })()) {}

	void set_proj_model_matrix(const glm::mat4 &m) {
		program->set_uniform("projection_view_matrix", m);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
