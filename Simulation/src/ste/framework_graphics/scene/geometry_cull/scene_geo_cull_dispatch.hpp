// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "Scene.hpp"
#include "light_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_geo_cull_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;
	const light_storage *ls;

	std::shared_ptr<Core::GLSLProgram> program;

	mutable std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs() const;

public:
	scene_geo_cull_dispatch(const StEngineControl &ctx,
							const Scene *scene,
							const light_storage *ls) : scene(scene), ls(ls),
													   program(ctx.glslprograms_pool().fetch_program_task({ "scene_geo_cull.glsl" })()) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
