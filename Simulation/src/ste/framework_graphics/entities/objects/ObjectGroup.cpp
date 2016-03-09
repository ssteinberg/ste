
#include "stdafx.h"
#include "ObjectGroup.h"

#include <vector>

using namespace StE::Graphics;

ObjectGroup::ObjectGroup(SceneProperties *props) : scene_props(props) {
	request_state({ GL_CULL_FACE, true });
	request_state({ GL_DEPTH_TEST, true });
  
	auto vbo_buffer = LLR::buffer_object_cast<vbo_type>(vbo.get_buffer());
	vao[0] = vbo_buffer[0];
	vao[1] = vbo_buffer[1];
	vao[2] = vbo_buffer[2];
	vao[3] = vbo_buffer[3];
}

void ObjectGroup::add_entity(const std::shared_ptr<Object> &entity) {
	auto &ind = entity->get_mesh().get_indices();
	auto &vertices = entity->get_mesh().get_vertices();

	entities.insert(std::make_pair(static_cast<int>(idb.size()), entity));

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

	entity->clear_model_mat_dirty_flag();
	entity->clear_material_id_dirty_flag();
	mesh_descriptor md;
	md.model = entity->get_model_transform();
	md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
	md.mat_idx = entity->get_material_id();
	mesh_data_bo.push_back(md);
}

void ObjectGroup::bind_buffers() const {
	using namespace LLR;
	vao.bind();
	LLR::buffer_object_cast<elements_type>(indices.get_buffer()).bind();
	LLR::buffer_object_cast<indirect_draw_buffer_type>(idb.get_buffer()).bind();
	0_storage_idx = scene_props->material_storage().buffer();
	1_storage_idx = mesh_data_bo.get_buffer();
}

void ObjectGroup::update_dirty_buffers() const {
	for (auto &p : entities) {
		if (p.second->is_model_mat_dirty() || p.second->is_material_id_dirty()) {
			range<> lock_range{ p.first * sizeof(mesh_data_buffer_type::T), sizeof(mesh_data_buffer_type::T) };

			mesh_descriptor md;
			md.model = p.second->get_model_transform();
			md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
			md.mat_idx = p.second->get_material_id();
			mesh_data_bo.overwrite(p.first, md);

			ranges_to_lock.push_back(lock_range);
			
			p.second->clear_model_mat_dirty_flag();
			p.second->clear_material_id_dirty_flag();
		}
	}
}

void ObjectGroup::prepare() const {
	renderable::prepare();
	bind_buffers();
	
	update_dirty_buffers();
}

void ObjectGroup::render() const {
	glMultiDrawElementsIndirect(GL_TRIANGLES, LLR::gl_type_name_enum<elements_type::T>::gl_enum, 0, idb.size(), 0);
}

void ObjectGroup::finalize() const {
	for (auto &r : ranges_to_lock)
		mesh_data_bo.lock_range(r);
	ranges_to_lock.clear();
}
