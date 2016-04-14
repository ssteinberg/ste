// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "ssss_storage.hpp"
#include "Scene.hpp"
#include "deferred_fbo.hpp"

#include "GLSLProgram.hpp"

#include "Quad.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_generator : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const ssss_storage *ssss;
	const Scene *scene;
	const deferred_fbo *deferred;

	std::shared_ptr<Core::GLSLProgram> ssss_gen_program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	ssss_generator(const StEngineControl &ctx,
				   const Scene *scene,
				   const ssss_storage *ssss,
				   const deferred_fbo *deferred) : ssss(ssss),
												   scene(scene),
												   deferred(deferred),
												   ssss_gen_program(ctx.glslprograms_pool().fetch_program_task({ "ssss.glsl" })()) {
		ssss_gen_program->set_uniform("far", ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float, float ffar) {
			ssss_gen_program->set_uniform("far", ffar);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
