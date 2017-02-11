
#include "stdafx.hpp"
#include "light_preprocessor.hpp"

#include "extract_projection_planes.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void light_preprocessor::set_projection_planes() const {
	glm::vec4 np;
	glm::vec4 fp;
	glm::vec4 rp;
	glm::vec4 lp;
	glm::vec4 tp;
	glm::vec4 bp;

	float aspect = ctx.get_projection_aspect();
	float fovy = ctx.get_fov();
	float fnear = ctx.get_near_clip();

	extract_projection_frustum_planes(fnear * 2, fnear, fovy, aspect,
									  &np, &fp, &rp, &lp, &tp, &bp);

	light_preprocess_cull_lights_program.get().set_uniform("np", np);
	light_preprocess_cull_lights_program.get().set_uniform("rp", rp);
	light_preprocess_cull_lights_program.get().set_uniform("lp", lp);
	light_preprocess_cull_lights_program.get().set_uniform("tp", tp);
	light_preprocess_cull_lights_program.get().set_uniform("bp", bp);
}

void light_preprocessor::set_context_state() const {
	ls->bind_lights_buffer(2);
	4_atomic_idx = ls->get_active_ll_counter();
	5_storage_idx = ls->get_active_ll();

	light_preprocess_cull_lights_program.get().bind();
}

void light_preprocessor::dispatch() const {
	constexpr int jobs = 128;
	auto size = (ls->size() + jobs - 1) / jobs;

	ls->clear_active_ll();
	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
}
