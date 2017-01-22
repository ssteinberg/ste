// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "ste_engine_control.hpp"

#include "glsl_program.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "scene.hpp"
#include "shadowmap_storage.hpp"

namespace StE {
namespace Graphics {

class directional_shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const scene *s;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	Resource::resource_instance<Resource::glsl_program> shadow_gen_program;

public:
	directional_shadowmap_projector(const ste_engine_control &ctx,
									const scene *s,
									light_storage *lights,
									const shadowmap_storage *shadow_map) : s(s),
																		   lights(lights),
																		   shadow_map(shadow_map),
																		   shadow_gen_program(ctx, std::vector<std::string>{ "shadow_directional.vert", "shadow_directional.geom" }) {
		shadow_gen_program.get().set_uniform("cascades_depths", s->properties().lights_storage().get_cascade_depths_array());
	}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
