
#include "stdafx.hpp"
#include "light_preprocess_cull_lights.hpp"

#include "light_storage.hpp"
#include "light_preprocessor.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void light_preprocess_cull_shadows::set_context_state() const {
	lp->ls->bind_lights_buffer(2);
	lp->ls->bind_lights_transform_buffer(3);
	4_storage_idx = Core::buffer_object_cast<Core::ShaderStorageBuffer<std::uint32_t>>(lp->ls->get_active_ll_counter());
	5_storage_idx = lp->ls->get_active_ll();

	lp->light_preprocess_cull_shadows_program->bind();
}

void light_preprocess_cull_shadows::dispatch() const {
	constexpr int jobs = 128;
	auto size = (max_active_lights_per_frame * 6 + jobs - 1) / jobs;

	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size, 1, 1);
}
