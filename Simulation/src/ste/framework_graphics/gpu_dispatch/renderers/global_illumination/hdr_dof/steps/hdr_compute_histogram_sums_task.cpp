
#include "stdafx.hpp"
#include "hdr_compute_histogram_sums_task.hpp"
#include "ShaderStorageBuffer.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void hdr_compute_histogram_sums_task::set_context_state() const {
	0_storage_idx = p->histogram_sums;
	1_storage_idx = buffer_object_cast<ShaderStorageBuffer<std::uint32_t>>(p->histogram);
	2_storage_idx = p->hdr_bokeh_param_buffer;

	p->gbuffer->bind_gbuffer();

	p->hdr_compute_histogram_sums->bind();
}

void hdr_compute_histogram_sums_task::dispatch() const {
	// p->hdr_compute_histogram_sums->set_uniform("time", p->ctx.time_per_frame().count());

	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	GL::gl_current_context::get()->dispatch_compute(1, 1, 1);
}
