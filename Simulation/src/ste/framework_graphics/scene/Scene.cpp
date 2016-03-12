
#include "stdafx.h"
#include "Scene.h"

#include "GLSLProgramFactory.h"

#include <vector>

using namespace StE::Graphics;

Scene::Scene(const StEngineControl &ctx) : object_program(ctx.glslprograms_pool().fetch_program_task({ "scene.vert", "scene.frag" })()) {
	request_state({ GL_CULL_FACE, true });
	request_state({ GL_DEPTH_TEST, true });
	
	object_program->set_uniform("projection", ctx.projection_matrix());
	object_program->set_uniform("far", ctx.get_far_clip());
	object_program->set_uniform("near", ctx.get_near_clip());
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float fnear, float ffar) {
		this->object_program->set_uniform("projection", proj);
		this->object_program->set_uniform("far", ffar);
		this->object_program->set_uniform("near", fnear);
	});
	ctx.signal_projection_change().connect(projection_change_connection);
}

void Scene::dispatch() const {
	auto model = entity_affine::get_model_transform();
	
	for (auto &obj : objects) {
		auto obj_model = obj->get_model_transform();
		auto m = model * obj_model;
		auto ti_m = glm::transpose(glm::inverse(m));
		
		object_program->set_uniform("view_matrix", m);
		object_program->set_uniform("trans_inverse_view_matrix", ti_m);
		
		(*obj)(object_program, Base::get_output_fbo());
	}
}
