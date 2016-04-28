
#include "stdafx.hpp"
#include "light_preprocess_dispatch.hpp"

#include "extract_projection_planes.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void light_preprocess_dispatch::set_projection_planes() const {
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

void light_preprocess_dispatch::set_context_state() const {
	ls->bind_lights_buffer(2);
	ls->bind_lights_transform_buffer(3);
	4_atomic_idx = ls->get_active_ll_counter();
	5_storage_idx = ls->get_active_ll();

	program->bind();
}

void light_preprocess_dispatch::dispatch() const {
	constexpr int jobs = 128;
	auto size = (ls->size() + jobs - 1) / jobs;

	ls->clear_active_ll();

	ls->update_storage();
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
	ls->lock_ranges();
}
