
#include <stdafx.hpp>
#include <object_group.hpp>

#include <mesh_descriptor.hpp>
#include <material.hpp>

#include <algorithm>

using namespace ste;
using namespace ste::graphics;

object_group::~object_group() {
	remove_all();
}

void object_group::add_object(gl::command_recorder &recorder,
							  const lib::shared_ptr<object> &obj) {
	// Attach connection for objec update
	auto connection = make_connection(obj->signal_model_change(), [this](object* obj) {
		this->signalled_objects.push_back(obj);
	});

	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

	assert(ind.size() && "Indices empty!");
	assert(vertices.size() && "Vertices empty!");

	// Add to objects set
	objects.insert(std::make_pair(obj,
								  object_information{ objects.size(), std::move(connection) }));

	// Append new vertex/index data
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_input,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(draw_buffers.get_vertex_buffer(),
																						gl::access_flags::vertex_attribute_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(draw_buffers.get_index_buffer(),
																						gl::access_flags::index_read,
																						gl::access_flags::transfer_write)));
	recorder << draw_buffers.get_vertex_buffer().push_back_cmd(vertices);
	recorder << draw_buffers.get_index_buffer().push_back_cmd(ind);
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_input,
															  gl::buffer_memory_barrier(draw_buffers.get_vertex_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::vertex_attribute_read),
															  gl::buffer_memory_barrier(draw_buffers.get_index_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::index_read)));

	auto transform_quat = obj->get_orientation();
	transform_quat.w *= -1.f;

	// Add a mesh descriptor for the new object
	mesh_descriptor md;
	md.model_transform_matrix() = glm::transpose(obj->get_model_transform());
	md.tangent_transform_quat() = transform_quat;
	md.mat_idx() = obj->get_material()->resource_index_in_storage();
	md.bounding_sphere() = obj->get_mesh().bounding_sphere().sphere();
	assert(md.mat_idx() >= 0);

	// And a draw parameters descriptor
	mesh_draw_params mdp;
	mdp.count() = static_cast<std::uint32_t>(ind.size());
	mdp.first_index() = total_indices;
	mdp.vertex_offset() = total_vertices;

	obj->md = md;

	// Append those too
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_input | gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_data_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_draw_params_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write)));
	recorder << draw_buffers.get_mesh_data_buffer().push_back_cmd(std::move(md));
	recorder << draw_buffers.get_mesh_draw_params_buffer().push_back_cmd(std::move(mdp));
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_input | gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_data_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_draw_params_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read)));

	total_vertices += static_cast<std::uint32_t>(vertices.size());
	total_indices += static_cast<std::uint32_t>(ind.size());
}

void object_group::remove_all() {
	for (auto &o : objects)
		o.first->signal_model_change().disconnect(&o.second.connection);
	objects.clear();
	signalled_objects.clear();
}

void object_group::update_dirty_buffers(gl::command_recorder &recorder) const {
	for (auto obj_ptr : signalled_objects) {
		const auto it = std::find_if(objects.begin(), objects.end(), [&](const objects_map_type::value_type &v) -> bool {
			return v.first.get() == obj_ptr;
		});
		if (it == objects.end()) {
			assert(false);
			continue;
		}
		auto &info = it->second;

		auto transform_quat = obj_ptr->get_orientation();
		transform_quat.w *= -1.f;

		mesh_descriptor md = obj_ptr->md;
		md.model_transform_matrix() = glm::transpose(obj_ptr->get_model_transform());
		md.tangent_transform_quat() = transform_quat;
		md.mat_idx() = obj_ptr->get_material()->resource_index_in_storage();
		assert(md.mat_idx() >= 0);

		recorder << draw_buffers.get_mesh_data_buffer().overwrite_cmd(info.index, md);
	}

	signalled_objects.clear();
}
