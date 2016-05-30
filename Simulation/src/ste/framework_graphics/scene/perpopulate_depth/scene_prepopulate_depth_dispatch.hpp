// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "glsl_program.hpp"
#include "Scene.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_prepopulate_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<scene_prepopulate_depth_dispatch>;

private:
	const Scene *scene;

	Resource::resource_instance<Resource::glsl_program> program;

public:
	scene_prepopulate_depth_dispatch(const StEngineControl &ctx, const Scene *scene) : scene(scene),
																					   program(ctx, std::vector<std::string>{ "scene_prepopulate_depth.vert", "scene_prepopulate_depth.frag" }) {}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::scene_prepopulate_depth_dispatch> {
	using R = Graphics::scene_prepopulate_depth_dispatch;

public:
	auto loader(const StEngineControl &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->program.wait();
		});
	}
};

}
}
