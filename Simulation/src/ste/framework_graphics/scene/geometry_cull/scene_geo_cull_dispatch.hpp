// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>
#include <gl_current_context.hpp>

#include <glsl_program.hpp>
#include <gpu_dispatchable.hpp>

#include <scene.hpp>
#include <light_storage.hpp>

#include <memory>

namespace ste {
namespace graphics {

class scene_geo_cull_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const scene *s;
	const light_storage *ls;

	resource::resource_instance<resource::glsl_program> program;

	mutable std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs() const;

public:
	scene_geo_cull_dispatch(const ste_engine_control &ctx,
							const scene *s,
							const light_storage *ls) : s(s),
													   ls(ls),
													   program(ctx, "scene_geo_cull.comp") {
		program.get().set_uniform("cascades_depths", s->properties().lights_storage().get_cascade_depths_array());
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
