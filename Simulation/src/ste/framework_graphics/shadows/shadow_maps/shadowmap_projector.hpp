// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "ObjectGroup.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const ObjectGroup *object;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	std::shared_ptr<Core::GLSLProgram> shadow_gen_program;

public:
	shadowmap_projector(const StEngineControl &ctx,
						const ObjectGroup *object,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : object(object),
															   lights(lights),
															   shadow_map(shadow_map),
															   shadow_gen_program(ctx.glslprograms_pool().fetch_program_task({ "shadow_map.vert", "shadow_cubemap.geom", "shadow_map.frag" })()) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
