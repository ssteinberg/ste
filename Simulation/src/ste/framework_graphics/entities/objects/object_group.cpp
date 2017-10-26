
#include <stdafx.hpp>
#include <object_group.hpp>

#include <mesh_descriptor.hpp>
#include <material.hpp>

#include <atomic>
#include <algorithm>

using namespace ste;
using namespace ste::graphics;

object_group::~object_group() {
	remove_all();
}

void object_group::add_object(const ste_context &ctx,
							  gl::command_recorder &recorder,
							  const lib::shared_ptr<object> &obj) {

	auto &ind = obj->get_mesh().get_indices();
	auto &vertices = obj->get_mesh().get_vertices();

	assert(ind.size() && "Indices empty!");
	assert(vertices.size() && "Vertices empty!");

	// Attach connection for object update
	auto connection = make_connection(obj->signal_model_change(), [this](object* obj) {
		this->signalled_objects.push_back(obj);
		std::atomic_thread_fence(std::memory_order_release);
	});

	auto transform_quat = obj->get_orientation();
	transform_quat.w *= -1.f;

	// Add a mesh descriptor for the new object
	mesh_descriptor md;
	md.model_transform_matrix() = glm::transpose(obj->get_model_transform());
	md.tangent_transform_quat() = transform_quat;
	md.mat_idx() = obj->get_material()->resource_index_in_storage();
	md.bounding_sphere() = obj->get_mesh().bounding_sphere().sphere();
	assert(md.mat_idx() >= 0);

	// Begin critical section
	std::unique_lock<std::mutex> l(m);

	// And a draw parameters descriptor
	mesh_draw_params mdp;
	mdp.count() = static_cast<std::uint32_t>(ind.size());
	mdp.first_index() = static_cast<std::uint32_t>(draw_buffers.get_index_buffer().size());
	mdp.vertex_offset() = static_cast<std::uint32_t>(draw_buffers.get_vertex_buffer().size());

	obj->md = md;
	// Add to objects set
	objects.insert(std::make_pair(obj,
								  object_information{ objects.size(), std::move(connection) }));
	object_sizes.push_back(mdp.count());

	// Append new vertex/index data
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_input | gl::pipeline_stage::draw_indirect | gl::pipeline_stage::vertex_shader | gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(draw_buffers.get_vertex_buffer(),
																						gl::access_flags::vertex_attribute_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(draw_buffers.get_index_buffer(),
																						gl::access_flags::index_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_data_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_draw_params_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write)));
	recorder << draw_buffers.get_vertex_buffer().push_back_cmd(ctx, vertices);
	recorder << draw_buffers.get_index_buffer().push_back_cmd(ctx, ind);
	recorder << draw_buffers.get_mesh_data_buffer().push_back_cmd(ctx, std::move(md));
	recorder << draw_buffers.get_mesh_draw_params_buffer().push_back_cmd(ctx, std::move(mdp));
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_input | gl::pipeline_stage::draw_indirect | gl::pipeline_stage::vertex_shader | gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_data_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(draw_buffers.get_mesh_draw_params_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(draw_buffers.get_vertex_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::vertex_attribute_read),
															  gl::buffer_memory_barrier(draw_buffers.get_index_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::index_read)));
}

void object_group::remove_all() {
	std::unique_lock<std::mutex> l(m);

	for (auto &o : objects)
		o.first->signal_model_change().disconnect(&o.second.connection);
	objects.clear();
	signalled_objects.clear();
}

void object_group::update_dirty_buffers(gl::command_recorder &recorder) const {
	// Create release->acquire memory semantics pair with the signalling lambda
	std::atomic_thread_fence(std::memory_order_acquire);
	if (!signalled_objects.size())
		return;

	std::unique_lock<std::mutex> l(m);

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
