
#include "stdafx.hpp"
#include "scene_frustum_cull_dispatch.hpp"

#include "Scene.hpp"
#include "extract_projection_planes.hpp"

using namespace StE::Graphics;

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

	program->bind();
}

void scene_frustum_cull_dispatch::dispatch() const {
	constexpr int jobs = 128;
	auto size = ( + jobs - 1) / jobs;

	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
}
