// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "Scene.hpp"
#include "shadowmap_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	const Scene *scene;
	light_storage *lights;
	const shadowmap_storage *shadow_map;

	std::shared_ptr<Core::GLSLProgram> shadow_gen_program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	shadowmap_projector(const StEngineControl &ctx,
						const Scene *scene,
						light_storage *lights,
						const shadowmap_storage *shadow_map) : scene(scene),
															   lights(lights),
															   shadow_map(shadow_map),
															   shadow_gen_program(ctx.glslprograms_pool().fetch_program_task({ "shadow_map.vert", "shadow_cubemap.geom" })()) {
		shadow_gen_program->set_uniform("far", 2.f * ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float, float ffar) {
			shadow_gen_program->set_uniform("far", 2.f * ffar);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
