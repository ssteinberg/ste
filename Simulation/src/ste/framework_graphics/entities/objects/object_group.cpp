// TODO
#include <stdafx.hpp>
//#include <object_group.hpp>
//
//#include <mesh_descriptor.hpp>
//#include <material.hpp>
//
//#include <gl_current_context.hpp>
//
//#include <algorithm>
//
//using namespace ste::graphics;
//
//object_group::~object_group() {
//	remove_all();
//}
//
//void object_group::add_object(const std::shared_ptr<object> &obj) {
//	auto connection = std::make_shared<signal_connection_type>(
//		[this](object* obj) {
//			this->signalled_objects.push_back(obj);
//		}
//	);
//	obj->signal_model_change().connect(connection);
//
//	auto &ind = obj->get_mesh().get_indices();
//	auto &vertices = obj->get_mesh().get_vertices();
//
//	assert(ind.size() && "Indices empty!");
//	assert(vertices.size() && "Vertices empty!");
//
//	objects.insert(std::make_pair(obj,
//								  object_information{ objects.size(), connection }));
//
//	draw_buffers.get_vbo_stack().push_back(vertices);
//	draw_buffers.get_indices_stack().push_back(ind);
//
//	auto transform_quat = obj->get_orientation();
//	transform_quat.w *= -1.f;
//
//	mesh_descriptor md;
//	md.model_transform_matrix = glm::transpose(obj->get_model_transform());
//	md.tangent_transform_quat = transform_quat;
//	md.mat_idx = obj->get_material()->resource_index_in_storage();
//	md.bounding_sphere = obj->get_mesh().bounding_sphere().sphere();
//	assert(md.mat_idx >= 0);
//
//	mesh_draw_params mdp;
//	mdp.count = ind.size();
//	mdp.first_index = total_indices;
//	mdp.base_vertex = total_vertices;
//
//	obj->md = md;
//
//	draw_buffers.get_mesh_data_stack().push_back(std::move(md));
//	draw_buffers.get_mesh_draw_params_stack().push_back(std::move(mdp));
//
//	total_vertices += vertices.size();
//	total_indices += ind.size();
//}
//
//void object_group::remove_all() {
//	for (auto &o : objects)
//		o.first->signal_model_change().disconnect(o.second.connection);
//	objects.clear();
//	signalled_objects.clear();
//}
//
//void object_group::update_dirty_buffers() const {
//	for (auto obj_ptr : signalled_objects) {
//		auto it = std::find_if(objects.begin(), objects.end(), [&](const objects_map_type::value_type &v) -> bool {
//			return v.first.get() == obj_ptr;
//		});
//		if (it == objects.end()) {
//			assert(false);
//			continue;
//		}
//		object_information info = it->second;
//
//		auto transform_quat = obj_ptr->get_orientation();
//		transform_quat.w *= -1.f;
//
//		mesh_descriptor md = obj_ptr->md;
//		md.model_transform_matrix = glm::transpose(obj_ptr->get_model_transform());
//		md.tangent_transform_quat = transform_quat;
//		md.mat_idx = obj_ptr->get_material()->resource_index_in_storage();
//		assert(md.mat_idx >= 0);
//
//		draw_buffers.get_mesh_data_stack().overwrite(info.index, md);
//	}
//
//	signalled_objects.clear();
//}
