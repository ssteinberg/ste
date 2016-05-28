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

private:
	const Scene *scene;

	Resource::resource_instance<Core::glsl_program> program;

private:
	scene_prepopulate_depth_dispatch(const Scene *scene) : scene(scene) {}

public:
	template <typename ... Ts>
	static auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<scene_prepopulate_depth_dispatch>(std::forward<Ts>(args)...);

			auto guard = object->program.load_and_wait_guard(ctx, std::vector<std::string>{ "scene_prepopulate_depth.vert", "scene_prepopulate_depth.frag" });

			return object;
		});
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

namespace Resource {

template <>
class resource_loading_task<deferred_composer> {
	using R = deferred_composer;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->program.wait();

			return object;
		});
	}
};

}
