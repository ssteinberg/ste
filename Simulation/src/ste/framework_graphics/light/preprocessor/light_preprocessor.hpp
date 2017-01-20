// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "ste_engine_control.hpp"
#include "gpu_task.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"

#include "signal.hpp"

#include "light_storage.hpp"
#include "glsl_program.hpp"

#include "light_preprocess_cull_lights.hpp"
#include "light_preprocess_cull_shadows.hpp"

namespace StE {
namespace Graphics {

class light_preprocessor {
	friend class light_preprocess_cull_lights;
	friend class light_preprocess_cull_shadows;

	friend class Resource::resource_loading_task<light_preprocessor>;
	friend class Resource::resource_instance<light_preprocessor>;

private:
	using ResizeSignalConnectionType = ste_engine_control::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = ste_engine_control::projection_change_signal_type::connection_type;

private:
	const ste_engine_control &ctx;
	light_storage *ls;

	light_preprocess_cull_lights stage1;
	light_preprocess_cull_shadows stage2;

	Resource::resource_instance<Resource::glsl_program> light_preprocess_cull_lights_program;
	Resource::resource_instance<Resource::glsl_program> light_preprocess_cull_shadows_program;

	std::shared_ptr<const gpu_task> task;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;
	void set_programs_cascades_depths_uniform() const {
		light_preprocess_cull_shadows_program.get().set_uniform("cascades_depths", ls->get_cascade_depths_array());
	}

private:
	light_preprocessor(const ste_engine_control &ctx,
					   light_storage *ls) : ctx(ctx), ls(ls),
											stage1(this), stage2(this),
											light_preprocess_cull_lights_program(ctx, "light_preprocess_cull_lights.glsl"),
											light_preprocess_cull_shadows_program(ctx, "light_preprocess_cull_shadows.glsl") {
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
	}

public:
	auto &get_task() const { return task; }
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::light_preprocessor> {
	using R = Graphics::light_preprocessor;

public:
	auto loader(const ste_engine_control &ctx, R* object) {
		return ctx.scheduler().schedule_now([object, &ctx]() {
			object->light_preprocess_cull_lights_program.wait();
			object->light_preprocess_cull_shadows_program.wait();
		}).then_on_main_thread([object]() {
			object->set_projection_planes();
			object->set_programs_cascades_depths_uniform();
		});
	}
};

}
}
