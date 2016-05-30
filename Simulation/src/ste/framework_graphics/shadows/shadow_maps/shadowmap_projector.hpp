// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "glsl_program.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "Scene.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<shadowmap_projector>;

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
															   shadow_gen_program(ctx, std::vector<std::string>{ "shadow_map.vert", "shadow_cubemap.geom" }) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::shadowmap_projector> {
	using R = Graphics::shadowmap_projector;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->shadow_gen_program.wait();
		});
	}
};

}
}
