
#include "stdafx.hpp"
#include "hdr_tonemap_coc_task.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void hdr_tonemap_coc_task::set_context_state() const {
	0_storage_idx = p->histogram_sums;
	2_storage_idx = p->hdr_bokeh_param_buffer;
	ScreenFillingQuad.vao()->bind();

	p->hdr_tonemap_coc->bind();
}

void hdr_tonemap_coc_task::dispatch() const {
	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
