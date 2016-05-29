// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

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

	Resource::resource_instance<Core::glsl_program> shadow_gen_program;

public:
	shadowmap_projector(const StEngineControl &ctx,
						const Scene *scene,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : scene(scene),
															   lights(lights),
															   shadow_map(shadow_map) {
		shadow_gen_program.load(ctx, std::vector<std::string>{ "shadow_map.vert", "shadow_cubemap.geom" });
	}

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
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, args...);

			object->shadow_gen_program.wait();

			return object;
		});
	}
};

}
}
