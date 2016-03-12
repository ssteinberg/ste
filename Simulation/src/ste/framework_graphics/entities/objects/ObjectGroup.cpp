
#include "stdafx.h"
#include "ObjectGroup.h"

#include <vector>
#include <algorithm>

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

ObjectGroup::~ObjectGroup() {
	remove_all();
}

void ObjectGroup::add_object(const std::shared_ptr<Object> &obj) {
	auto connection = std::make_shared<signal_connection_type>(
		[this](Object* obj) {
			this->signalled_objects.push_back(obj);
		}
	);
	obj->signal_model_change().connect(connection);
	
	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

	objects.insert(std::make_pair(obj, 
								  object_information{ idb.size(), connection }));

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

	mesh_descriptor md;
	md.model = obj->get_model_transform();
	md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
	md.mat_idx = obj->get_material_id();
	mesh_data_bo.push_back(md);
}

void ObjectGroup::remove_all() {
	for (auto &o : objects)
		o.first->signal_model_change().disconnect(o.second.connection);
	objects.clear();
	signalled_objects.clear();
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
	for (auto obj_ptr : signalled_objects) {
		auto it = std::find_if(objects.begin(), objects.end(), [&](const objects_map_type::value_type &v) -> bool {
			return v.first.get() == obj_ptr;
		});
		if (it == objects.end()) {
			assert(false);
			continue;
		}
		object_information info = it->second;
		
		range<> lock_range{ info.index * sizeof(mesh_data_buffer_type::T), sizeof(mesh_data_buffer_type::T) };

		mesh_descriptor md;
		md.model = obj_ptr->get_model_transform();
		md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
		md.mat_idx = obj_ptr->get_material_id();
		mesh_data_bo.overwrite(info.index, md);

		ranges_to_lock.push_back(lock_range);
	}
	
	signalled_objects.clear();
}

void ObjectGroup::prepare() const {
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
