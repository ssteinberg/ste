// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "shadowmap_storage.hpp"

#include "ssss_storage.hpp"
#include "Scene.hpp"
#include "deferred_gbuffer.hpp"

#include "Quad.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_bilateral_blur_x;
class ssss_bilateral_blur_y;
class ssss_write_penumbras;

class ssss_generator {
	friend class ssss_bilateral_blur_x;
	friend class ssss_bilateral_blur_y;
	friend class ssss_write_penumbras;

	friend class Resource::resource_loading_task<ssss_generator>;

private:
	const shadowmap_storage *shadows_storage;
	const ssss_storage *ssss;
	const Scene *scene;
	const deferred_gbuffer *gbuffer;

	Resource::resource_instance<ssss_bilateral_blur_x> bilateral_blur_x;
	Resource::resource_instance<ssss_bilateral_blur_y> bilateral_blur_y;
	Resource::resource_instance<ssss_write_penumbras> write_penumbras;

	std::shared_ptr<const gpu_task> task;

private:
	ssss_generator(const StEngineControl &ctx,
				   const Scene *scene,
				   const shadowmap_storage *shadows_storage,
				   const ssss_storage *ssss,
				   const deferred_gbuffer *gbuffer);

public:
	~ssss_generator() noexcept;

	auto get_task() const { return task; }
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::ssss_generator> {
	using R = Graphics::ssss_generator;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, const Ts&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, args...);

			auto blur_x_task = make_gpu_task("ssss_blur_x", object->bilateral_blur_x.get(), nullptr);
			auto write_task = make_gpu_task("ssss_write_penumbras", object->write_penumbras.get(), nullptr);
			blur_x_task->add_dependency(write_task);

			object->task = make_gpu_task("ssss_blur_y", object->bilateral_blur_y.get(), nullptr, { blur_x_task, write_task });

			return object;
		});
	}
};

}
}
