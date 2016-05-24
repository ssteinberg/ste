
#include "stdafx.hpp"
#include "light_preprocess_cull_lights.hpp"

#include "light_preprocessor.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void light_preprocess_cull_lights::set_context_state() const {
	lp->ls->bind_lights_buffer(2);
	4_atomic_idx = lp->ls->get_active_ll_counter();
	5_storage_idx = lp->ls->get_active_ll();

	lp->light_preprocess_cull_lights_program->bind();
}

void light_preprocess_cull_lights::dispatch() const {
	constexpr int jobs = 128;
	auto size = (lp->ls->size() + jobs - 1) / jobs;

	lp->ls->clear_active_ll();
	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
}
