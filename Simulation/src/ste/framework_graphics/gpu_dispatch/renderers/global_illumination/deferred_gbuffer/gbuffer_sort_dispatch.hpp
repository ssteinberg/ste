// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "deferred_gbuffer.hpp"
#include "glsl_program.hpp"

namespace StE {
namespace Graphics {

class gbuffer_sort_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	deferred_gbuffer *gbuffer;
	Resource::resource_instance<Core::glsl_program> sort_program;

private:
	gbuffer_sort_dispatch(deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {}

public:
	static auto loader(const StEngineControl &ctx, deferred_gbuffer *gbuffer) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<gbuffer_sort_dispatch>(gbuffer);

			auto guard = object->sort_program.load_and_wait_guard(ctx, "gbuffer_sort.glsl");

			return object;
		});
	}

protected:
	virtual void set_context_state() const override {
		gbuffer->bind_gbuffer();
		sort_program.get().bind();
	}

	virtual void dispatch() const override {
		constexpr int jobs = 32;
		auto size = (gbuffer->get_size() + glm::ivec2(jobs - 1)) / jobs;

		Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
	}
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
