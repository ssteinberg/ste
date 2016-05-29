// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

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

	Resource::resource_instance<Core::glsl_program> program;

public:
	scene_prepopulate_depth_dispatch(const StEngineControl &ctx, const Scene *scene) : scene(scene) {
		program.load(ctx, std::vector<std::string>{ "scene_prepopulate_depth.vert", "scene_prepopulate_depth.frag" });
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::scene_prepopulate_depth_dispatch> {
	using R = Graphics::scene_prepopulate_depth_dispatch;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, args...);

			object->program.wait();

			return object;
		});
	}
};

}
}
