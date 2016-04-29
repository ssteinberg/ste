// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"
#include "ObjectGroup.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene;

class scene_frustum_cull_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const StEngineControl &ctx;
	const Scene *scene;

	std::shared_ptr<Core::GLSLProgram> program;

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void set_projection_planes() const;

public:
	scene_frustum_cull_dispatch(const StEngineControl &ctx,
								const Scene *scene) : ctx(ctx), scene(scene),
													  program(ctx.glslprograms_pool().fetch_program_task({ "scene_frustum_cull.glsl" })()) {
		set_projection_planes();
		resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
			set_projection_planes();
		});
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float n, float f) {
			set_projection_planes();
		});
		ctx.signal_framebuffer_resize().connect(resize_connection);
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		program->set_uniform("view_matrix", m);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
