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

	friend class Resource::resource_loading_task<gbuffer_sort_dispatch>;

private:
	deferred_gbuffer *gbuffer;
	Resource::resource_instance<Core::glsl_program> sort_program;

public:
	gbuffer_sort_dispatch(const StEngineControl &ctx, deferred_gbuffer *gbuffer) : gbuffer(gbuffer) {
		sort_program.load(ctx, "gbuffer_sort.glsl");
	}

protected:
	void set_context_state() const override final {
		gbuffer->bind_gbuffer();
		sort_program.get().bind();
	}

	void dispatch() const override final {
		constexpr int jobs = 32;
		auto size = (gbuffer->get_size() + glm::ivec2(jobs - 1)) / jobs;

		Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
	}
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::gbuffer_sort_dispatch> {
	using R = Graphics::gbuffer_sort_dispatch;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, args...);

			object->sort_program.wait();

			return object;
		});
	}
};

}
}
