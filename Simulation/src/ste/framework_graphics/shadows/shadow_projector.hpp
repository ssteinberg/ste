// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gl_current_context.hpp"

#include "GLSLProgram.hpp"

#include "gpu_dispatchable.hpp"
#include "ObjectGroup.hpp"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

class shadow_projector : public gpu_dispatchable {
private:
	ObjectGroup *object;

	std::shared_ptr<Core::GLSLProgram> shadow_gen_program;

	glm::mat4 shadow_proj;

public:
	shadow_projector(StEngineControl &ctx, ObjectGroup *object) : object(object), shadow_gen_program(ctx.glslprograms_pool().fetch_program_task({ "shadow_map.vert", "shadow_cubemap.geom", "shadow_map.frag" })()) {
		shadow_proj = glm::perspective(glm::half_pi<float>(), 1.f, 20.f, 3000.f);

		shadow_gen_program->set_uniform("shadow_transforms", std::vector<glm::mat4>{
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f,-1.f, 0.f), glm::vec3(0.f, 0.f,-1.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f, 1.f), glm::vec3(0.f,-1.f, 0.f)),
			shadow_proj * glm::lookAt(glm::vec3(0), glm::vec3( 0.f, 0.f,-1.f), glm::vec3(0.f,-1.f, 0.f))
		});
	}

	void set_light_pos(const glm::vec3 &lp) const {
		shadow_gen_program->set_uniform("light_pos", glm::vec4(lp.x, lp.y, lp.z, .0f));
	}

	void set_context_state() const override final {
		Core::gl_current_context::get()->viewport(0,0,1024,1024);
		object->set_context_state();
		shadow_gen_program->bind();
	}

	void dispatch() const override final {
		object->dispatch();
	}
};

}
}
