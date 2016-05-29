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

	friend class Resource::resource_loading_task<scene_geo_cull_dispatch>;

private:
	const Scene *scene;
	const light_storage *ls;

	Resource::resource_instance<Core::glsl_program> program;

	mutable std::size_t old_object_group_size{ 0 };

private:
	void commit_idbs() const;

public:
	scene_geo_cull_dispatch(const StEngineControl &ctx,
							const Scene *scene,
							const light_storage *ls) : scene(scene), ls(ls) {
		program.load(ctx, "scene_geo_cull.glsl");
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::scene_geo_cull_dispatch> {
	using R = Graphics::scene_geo_cull_dispatch;

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
