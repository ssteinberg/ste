
#include "stdafx.hpp"
#include "scene_frustum_cull_dispatch.hpp"

#include "Scene.hpp"
#include "extract_projection_planes.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void scene_frustum_cull_dispatch::set_context_state() const {
	auto& draw_buffers = scene->object_group().get_draw_buffers();

	0_atomic_idx = draw_buffers.get_culled_objects_counter();
 	0_storage_idx = draw_buffers.get_culled_indirect_command_buffer();
	draw_buffers.get_mesh_data_buffer().bind_range(Core::shader_storage_layout_binding(1), 0, draw_buffers.size());

	ls->bind_lights_buffer(2);
	ls->bind_lights_transform_buffer(3);
	4_storage_idx = Core::buffer_object_cast<Core::ShaderStorageBuffer<std::uint32_t>>(ls->get_active_ll_counter());
	5_storage_idx = ls->get_active_ll();

	program->bind();
}

void scene_frustum_cull_dispatch::dispatch() const {
	auto& draw_buffers = scene->object_group().get_draw_buffers();
	constexpr int jobs = 128;
	auto size = (draw_buffers.size() + jobs - 1) / jobs;

	draw_buffers.clear_indirect_command_buffer();
	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);

	scene->object_group().update_dirty_buffers();
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
	scene->object_group().lock_updated_buffers();
}
