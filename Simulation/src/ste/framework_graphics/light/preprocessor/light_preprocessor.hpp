// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_task.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "signal.hpp"

#include "light_storage.hpp"
#include "hdr_dof_postprocess.hpp"
#include "glsl_program.hpp"

#include "light_preprocess_cull_lights.hpp"
#include "light_preprocess_cull_shadows.hpp"

namespace StE {
namespace Graphics {

class light_preprocessor {
	friend class light_preprocess_cull_lights;
	friend class light_preprocess_cull_shadows;

	friend class Resource::resource_loading_task<light_preprocessor>;

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const StEngineControl &ctx;
	light_storage *ls;
	const hdr_dof_postprocess *hdr;

	light_preprocess_cull_lights stage1;
	light_preprocess_cull_shadows stage2;

	Resource::resource_instance<Core::glsl_program> light_preprocess_cull_lights_program;
	Resource::resource_instance<Core::glsl_program> light_preprocess_cull_shadows_program;

	std::shared_ptr<const gpu_task> task;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;

public:
	light_preprocessor(const StEngineControl &ctx,
					   light_storage *ls,
					   const hdr_dof_postprocess *hdr) : ctx(ctx), ls(ls), hdr(hdr),
														 stage1(this), stage2(this) {
		set_projection_planes();
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			set_projection_planes();
		});
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](float, float, float n) {
			set_projection_planes();
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
		ctx.signal_projection_change().connect(projection_change_connection);

		auto stage1_task = make_gpu_task("light_preprocessor_stage1", &stage1, nullptr);
		task = make_gpu_task("light_preprocessor", &stage2, nullptr, { stage1_task });

		light_preprocess_cull_lights_program.load(ctx, "light_preprocess_cull_lights.glsl");
		light_preprocess_cull_shadows_program.load(ctx, "light_preprocess_cull_shadows.glsl");
	}

	auto &get_task() const { return task; }
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::light_preprocessor> {
	using R = Graphics::light_preprocessor;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->light_preprocess_cull_lights_program.wait();
			object->light_preprocess_cull_shadows_program.wait();

			return object;
		});
	}
};

}
}
