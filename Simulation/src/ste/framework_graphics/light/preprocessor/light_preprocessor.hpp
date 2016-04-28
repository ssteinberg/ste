// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_task.hpp"

#include "signal.hpp"

#include "light_storage.hpp"
#include "GLSLProgram.hpp"

#include "light_preprocess_cull_lights.hpp"
#include "light_preprocess_cull_shadows.hpp"

namespace StE {
namespace Graphics {

class light_preprocessor {
	friend class light_preprocess_cull_lights;
	friend class light_preprocess_cull_shadows;

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const StEngineControl &ctx;
	light_storage *ls;

	light_preprocess_cull_lights stage1;
	light_preprocess_cull_shadows stage2;

	std::shared_ptr<Core::GLSLProgram> light_preprocess_cull_lights_program;
	std::shared_ptr<Core::GLSLProgram> light_preprocess_cull_shadows_program;

	std::shared_ptr<const gpu_task> task;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;

public:
	light_preprocessor(const StEngineControl &ctx, light_storage *ls) : ctx(ctx), ls(ls),
																		stage1(this),
																		stage2(this),
																		light_preprocess_cull_lights_program(ctx.glslprograms_pool().fetch_program_task({ "light_preprocess_cull_lights.glsl" })()),
																		light_preprocess_cull_shadows_program(ctx.glslprograms_pool().fetch_program_task({ "light_preprocess_cull_shadows.glsl" })()) {
		set_projection_planes();
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			set_projection_planes();
		});
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float n, float f) {
			set_projection_planes();
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
		ctx.signal_projection_change().connect(projection_change_connection);

		auto stage1_task = make_gpu_task("light_preprocessor_stage1", &stage1, nullptr);;
		task = make_gpu_task("light_preprocessor", &stage2, nullptr, { stage1_task });
	}

	void set_model_matrix(const glm::mat4 &m) {
		light_preprocess_cull_lights_program->set_uniform("view_matrix", m);
		light_preprocess_cull_shadows_program->set_uniform("view_matrix", m);
	}

	auto &get_task() const { return task; }
};

}
}
