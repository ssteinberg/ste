
#include "stdafx.hpp"
#include "scene_frustum_cull_dispatch.hpp"

#include "Scene.hpp"
#include "extract_projection_planes.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void scene_frustum_cull_dispatch::set_projection_planes() const {
	glm::vec4 np;
	glm::vec4 fp;
	glm::vec4 rp;
	glm::vec4 lp;
	glm::vec4 tp;
	glm::vec4 bp;

	float aspect = ctx.get_projection_aspect();
	float fovy = ctx.get_fov();
	float far = ctx.get_far_clip();
	float near = ctx.get_near_clip();

	extract_projection_frustum_planes(far, near, fovy, aspect,
									  &np, &fp, &rp, &lp, &tp, &bp);

	program->set_uniform("np", np);
	program->set_uniform("fp", fp);
	program->set_uniform("rp", rp);
	program->set_uniform("lp", lp);
	program->set_uniform("tp", tp);
	program->set_uniform("bp", bp);
}

void scene_frustum_cull_dispatch::set_context_state() const {
	auto& draw_buffers = scene->object_group().get_draw_buffers();

	0_atomic_idx = draw_buffers.get_culled_objects_counter();
 	0_storage_idx = draw_buffers.get_culled_indirect_command_buffer();
	draw_buffers.get_mesh_data_buffer().bind_range(Core::shader_storage_layout_binding(1), 0, draw_buffers.size());
	12_storage_idx = draw_buffers.get_id_to_drawid_buffer();

	program->bind();
}

void scene_frustum_cull_dispatch::dispatch() const {
	auto& draw_buffers = scene->object_group().get_draw_buffers();
	constexpr int jobs = 128;
	auto size = (draw_buffers.size() + jobs - 1) / jobs;

	draw_buffers.clear_indirect_command_buffer();

	scene->object_group().update_dirty_buffers();
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
	scene->object_group().lock_updated_buffers();
}
