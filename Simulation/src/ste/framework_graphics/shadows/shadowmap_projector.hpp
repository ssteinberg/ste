// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "signal.hpp"

#include "pinned_gvector.hpp"
#include "GLSLProgram.hpp"
#include "gpu_dispatchable.hpp"

#include "light_storage.hpp"
#include "ObjectGroup.hpp"
#include "shadowmap_storage.hpp"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

class shadowmap_projector : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	StEngineControl &ctx;

	ObjectGroup *object;
	light_storage *lights;
	shadowmap_storage *shadow_map;

	std::shared_ptr<Core::GLSLProgram> shadow_gen_program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void update_transforms() const {
		auto shadow_proj = glm::perspective(glm::half_pi<float>(), 1.f, 20.f, ctx.get_far_clip());

		shadow_gen_program->set_uniform("far", ctx.get_far_clip());
		shadow_gen_program->set_uniform("shadow_transforms", std::vector<glm::mat4>{
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f,-1.f, 0.f), glm::vec3(0.f, 0.f,-1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f, 1.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f,-1.f), glm::vec3(0.f,-1.f, 0.f))
		});
	}

public:
	shadowmap_projector(StEngineControl &ctx, ObjectGroup *object, light_storage *lights, shadowmap_storage *shadow_map) : ctx(ctx),
																							object(object),
																							lights(lights),
																							shadow_map(shadow_map),
																							shadow_gen_program(ctx.glslprograms_pool().fetch_program_task({ "shadow_map.vert", "shadow_cubemap.geom", "shadow_map.frag" })()) {
		update_transforms();
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([this](const glm::mat4&, float, float, float ffar) {
			this->update_transforms();
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_context_state() const override final {
		object->set_context_state();

		auto size = shadow_map->get_cubemaps()->get_size();
		Core::gl_current_context::get()->viewport(0, 0, size.x, size.y);

		lights->bind_buffers(2);
		shadow_gen_program->bind();
	}

	void dispatch() const override final {
		Core::gl_current_context::get()->clear_framebuffer(false, true);

		lights->update_storage();

		object->dispatch();

		lights->lock_ranges();
	}
};

}
}
