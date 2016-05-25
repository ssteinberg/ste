// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgramFactory.hpp"
#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "Scene.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	std::shared_ptr<Core::GLSLProgram> shadow_gen_program;

public:
	shadowmap_projector(const StEngineControl &ctx,
						const Scene *scene,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : scene(scene),
															   lights(lights),
															   shadow_map(shadow_map),
															   shadow_gen_program(Resource::GLSLProgramFactory::load_program_task(ctx, { "shadow_map.vert", "shadow_cubemap.geom" })()) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
