// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "glsl_program.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "Scene.hpp"
#include "shadowmap_storage.hpp"

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	Resource::resource_instance<Resource::glsl_program> shadow_gen_program;

public:
	shadowmap_projector(const StEngineControl &ctx,
						const Scene *scene,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : scene(scene),
															   lights(lights),
															   shadow_map(shadow_map),
															   shadow_gen_program(ctx, std::vector<std::string>{ "shadow_cubemap.vert", "shadow_cubemap.geom" }) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
