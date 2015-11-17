
#include "stdafx.h"
#include "Scene.h"
#include "Log.h"

#include "gl_type_traits.h"

#include "GLSLProgramLoader.h"

#include <vector>

using namespace StE::Graphics;

Scene::Scene(const StEngineControl &ctx, SceneProperties *props) : renderable(StE::Resource::GLSLProgramLoader::load_program_task(ctx, { "scene.vert", "scene.frag" })()), scene_props(props) {
	request_state({ GL_CULL_FACE, true });
	request_state({ GL_DEPTH_TEST, true });
  
  	vao = std::make_unique<LLR::VertexArrayObject>();
	auto vbo_buffer = LLR::buffer_object_cast<vbo_type>(vbo.get_buffer());
	(*vao)[0] = vbo_buffer[0];
	(*vao)[1] = vbo_buffer[1];
	(*vao)[2] = vbo_buffer[2];
	(*vao)[3] = vbo_buffer[3];

	get_program()->set_uniform("projection", ctx.projection_matrix());
	get_program()->set_uniform("far", ctx.get_far_clip());
	get_program()->set_uniform("near", ctx.get_near_clip());
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float fnear, float ffar) {
		this->get_program()->set_uniform("projection", proj);
		this->get_program()->set_uniform("far", ffar);
		this->get_program()->set_uniform("near", fnear);
	});
	ctx.signal_projection_change().connect(projection_change_connection);
}

void Scene::add_object(const std::shared_ptr<Object> &obj) {
	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

	objects.insert(std::make_pair(static_cast<int>(idb.size()), obj));

 	LLR::IndirectMultiDrawElementsCommand idc;
 	idc.count = ind.size();
 	idc.instance_count = 1;
 	idc.first_index = total_indices;
 	idc.base_vertex = total_vertices;
 	idc.base_instance = 0;

	vbo.push_back(vertices);
	indices.push_back(ind);
	idb.push_back(idc);
 
 	total_vertices += vertices.size();
 	total_indices += ind.size();

	obj->clear_dirty();
	mesh_descriptor md;
	md.model = obj->get_model_transform();
	md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
	md.mat_idx = obj->get_material_id();
	mesh_data_bo.push_back(md);
}

void Scene::prepare() const {
 	using namespace LLR;

	for (auto &p : objects) {
		if (p.second->is_dirty()) {
			p.second->clear_dirty();

			range<> lock_range{ p.first * sizeof(mesh_data_buffer_type::T), sizeof(mesh_data_buffer_type::T) };

			mesh_descriptor md;
			md.model = p.second->get_model_transform();
			md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
			md.mat_idx = p.second->get_material_id();
			mesh_data_bo.overwrite(p.first, md);

			ranges_to_lock.push_back(lock_range);
		}
	}
 
	renderable::prepare();
 	vao->bind();
	LLR::buffer_object_cast<elements_type>(indices.get_buffer()).bind();
	LLR::buffer_object_cast<indirect_draw_buffer_type>(idb.get_buffer()).bind();
	0_storage_idx = scene_props->material_storage().buffer();
	1_storage_idx = mesh_data_bo.get_buffer();
}

void Scene::render() const {
	glMultiDrawElementsIndirect(GL_TRIANGLES, LLR::gl_type_name_enum<elements_type::T>::gl_enum, nullptr, idb.size(), 0);
}

void Scene::finalize() const {
	for (auto &r : ranges_to_lock)
		mesh_data_bo.lock_range(r);
	ranges_to_lock.clear();
}
