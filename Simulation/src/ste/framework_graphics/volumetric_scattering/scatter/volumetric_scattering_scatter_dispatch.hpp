// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "volumetric_scattering_storage.hpp"
#include "linked_light_lists.hpp"
#include "light_storage.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_scatter_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const volumetric_scattering_storage *vss;
	const linked_light_lists *llls;
	const light_storage *ls;
	const shadowmap_storage *shadows_storage;

	std::shared_ptr<Core::GLSLProgram> program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void update_shader_proj_uniforms(const glm::mat4 &projection) {
		float proj00 = projection[0][0];
		float proj11 = projection[1][1];
		float proj23 = projection[3][2];

		program->set_uniform("proj00", proj00);
		program->set_uniform("proj11", proj11);
		program->set_uniform("proj23", proj23);
	}

public:
	volumetric_scattering_scatter_dispatch(const StEngineControl &ctx,
										   const volumetric_scattering_storage *vss,
										   const linked_light_lists *llls,
										   const light_storage *ls,
										   const shadowmap_storage *shadows_storage) : vss(vss), llls(llls), ls(ls), shadows_storage(shadows_storage),
																					   program(ctx.glslprograms_pool().fetch_program_task({ "volumetric_scattering_scatter.glsl" })()) {
		update_shader_proj_uniforms(ctx.projection_matrix());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4 &proj, float, float n) {
			update_shader_proj_uniforms(proj);
		});
		ctx.signal_projection_change().connect(projection_change_connection);

		shadowmap_storage::update_shader_shadow_proj_uniforms(program.get());
	}

	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
