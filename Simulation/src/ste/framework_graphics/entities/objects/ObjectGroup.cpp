
#include "stdafx.hpp"
#include "ObjectGroup.hpp"

#include "mesh_descriptor.hpp"

#include "glsl_programs_pool.hpp"
#include "gl_current_context.hpp"

#include <algorithm>

using namespace StE::Graphics;

ObjectGroup::ObjectGroup(const StEngineControl &ctx,
						 const SceneProperties *props) : scene_props(props),
														 object_program(ctx.glslprograms_pool().fetch_program_task({ "object.vert", "object.frag" })()) {
	object_program->set_uniform("projection", ctx.projection_matrix());
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float fnear, float ffar) {
		this->object_program->set_uniform("projection", proj);
	});
	ctx.signal_projection_change().connect(projection_change_connection);
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
								  object_information{ objects.size(), connection }));

 	draw_buffers.get_culled_indirect_command_stack().push_back(Core::IndirectMultiDrawElementsCommand());
	draw_buffers.get_id_to_drawid_stack().push_back(0);
	draw_buffers.get_vbo_stack().push_back(vertices);
	draw_buffers.get_indices_stack().push_back(ind);

	mesh_descriptor md;
	md.model = obj->get_model_transform();
	md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
	md.mat_idx = obj->get_material_id();
	md.bounding_sphere = obj->get_mesh().bounding_sphere().sphere();
 	md.count = ind.size();
 	md.first_index = total_indices;
 	md.base_vertex = total_vertices;

	obj->md = md;

	draw_buffers.get_mesh_data_stack().push_back(std::move(md));

 	total_vertices += vertices.size();
 	total_indices += ind.size();
}

void ObjectGroup::remove_all() {
	for (auto &o : objects)
		o.first->signal_model_change().disconnect(o.second.connection);
	objects.clear();
	signalled_objects.clear();
}

void ObjectGroup::bind_buffers() const {
	using namespace Core;

	draw_buffers.get_vao().bind();
	draw_buffers.get_elements_buffer().bind();
	draw_buffers.get_culled_indirect_draw().bind();
	0_storage_idx = scene_props->materials_storage().buffer();
	1_storage_idx = draw_buffers.get_mesh_data_buffer();
	12_storage_idx = draw_buffers.get_id_to_drawid_buffer();
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

		range<> lock_range{ info.index * sizeof(object_group_draw_buffers::mesh_data_buffer_type::T),
							sizeof(object_group_draw_buffers::mesh_data_buffer_type::T) };

		mesh_descriptor md = obj_ptr->md;
		md.model = obj_ptr->get_model_transform();
		md.transpose_inverse_model = glm::transpose(glm::inverse(md.model));
		md.mat_idx = obj_ptr->get_material_id();
		draw_buffers.get_mesh_data_stack().overwrite(info.index, md);

		ranges_to_lock.push_back(lock_range);
	}

	signalled_objects.clear();
}

void ObjectGroup::lock_updated_buffers() const {
	for (auto &r : ranges_to_lock)
		draw_buffers.get_mesh_data_stack().lock_range(r);
	ranges_to_lock.clear();
}

void ObjectGroup::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->depth_func(GL_LEQUAL);
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->depth_mask(false);

	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);

	gbuffer->bind_gbuffer(false);

	bind_buffers();
	object_program->bind();
}

void ObjectGroup::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_multi_elements_indirect<object_group_draw_buffers::elements_type::T>(GL_TRIANGLES, 0, draw_buffers.size(), 0);
}
