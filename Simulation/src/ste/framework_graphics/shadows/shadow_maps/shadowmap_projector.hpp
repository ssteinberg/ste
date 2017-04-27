// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>

#include <glsl_program.hpp>
#include <gpu_dispatchable.hpp>

#include <light_storage.hpp>
#include <scene.hpp>
#include <shadowmap_storage.hpp>

namespace ste {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const scene *s;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	resource::resource_instance<resource::glsl_program> shadow_gen_program;

public:
	shadowmap_projector(const ste_engine_control &ctx,
						const scene *s,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : s(s),
															   lights(lights),
															   shadow_map(shadow_map),
															   shadow_gen_program(ctx, std::vector<std::string>{ "shadow_cubemap.vert", "shadow_cubemap.geom" }) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
