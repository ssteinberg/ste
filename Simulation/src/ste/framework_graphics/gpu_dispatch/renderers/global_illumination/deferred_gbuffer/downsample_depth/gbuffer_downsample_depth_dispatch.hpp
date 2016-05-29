// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "glsl_program.hpp"
#include "deferred_gbuffer.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class gbuffer_downsample_depth_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const deferred_gbuffer *gbuffer;

	Resource::resource_instance<Core::glsl_program> program;

private:
	gbuffer_downsample_depth_dispatch(const StEngineControl &ctx,
									  const deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {}

public:
	static auto loader(const StEngineControl &ctx, const deferred_gbuffer *gbuffer) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<gbuffer_downsample_depth_dispatch>(ctx, gbuffer);

			auto guard = object->program.load_and_wait_guard(ctx, "gbuffer_downsample_depth.glsl");

			return object;
		});
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}

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
