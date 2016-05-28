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

#include "Scene.hpp"
#include "light_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class scene_geo_cull_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const Scene *scene;
	const light_storage *ls;

	Resource::resource_instance<Core::glsl_program> program;

	mutable std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs() const;

private:
	scene_geo_cull_dispatch(const Scene *scene,
							const light_storage *ls) : scene(scene), ls(ls) {}

public:
	template <typename ... Ts>
	static auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<scene_geo_cull_dispatch>(std::forward<Ts>(args)...);

			auto guard = object->program.load_and_wait_guard(ctx, "scene_geo_cull.glsl");

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
