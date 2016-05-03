// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "ssss_generator.hpp"

#include "GLSLProgram.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ssss_write_penumbras : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const ssss_generator *p;
	std::shared_ptr<Core::GLSLProgram> ssss_gen_program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	ssss_write_penumbras(const ssss_generator *p, const StEngineControl &ctx) : p(p),
																				ssss_gen_program(ctx.glslprograms_pool().fetch_program_task({ "ssss.glsl" })()) {
		ssss_gen_program->set_uniform("far", ctx.get_far_clip());
		ssss_gen_program->set_uniform("near", ctx.get_near_clip());
		ssss_gen_program->set_uniform("half_over_tan_fov_over_two", .5f / glm::tan(ctx.get_fov() * .5f));
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float ffov, float fnear, float ffar) {
			ssss_gen_program->set_uniform("far", ffar);
			ssss_gen_program->set_uniform("near", fnear);
			ssss_gen_program->set_uniform("half_over_tan_fov_over_two", .5f / glm::tan(ffov * .5f));
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
