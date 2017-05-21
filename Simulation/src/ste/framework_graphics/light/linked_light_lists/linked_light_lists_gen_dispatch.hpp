// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>
#include <gpu_dispatchable.hpp>

#include <glsl_program.hpp>

#include <linked_light_lists.hpp>
#include <light_storage.hpp>

#include <glsl_program.hpp>
#include <texture_2d.hpp>
#include <Sampler.hpp>

namespace ste {
namespace graphics {

class linked_light_lists_gen_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	light_storage *ls;
	linked_light_lists *lll;
	resource::resource_instance<resource::glsl_program> program;

public:
	linked_light_lists_gen_dispatch(const ste_engine_control &ctx,
									light_storage *ls,
									linked_light_lists *lll) : ls(ls), lll(lll),
															   program(ctx, lib::vector<lib::string>{ "linked_light_lists_gen.comp" }) {}

	void set_depth_map(Core::texture_2d *depth_map) {
		auto handle = depth_map->get_texture_handle();
		handle.make_resident();
		program.get().set_uniform("depth_map", handle);
	}

protected:
	virtual void set_context_state() const override;
	virtual void dispatch() const override;
};

}
}
