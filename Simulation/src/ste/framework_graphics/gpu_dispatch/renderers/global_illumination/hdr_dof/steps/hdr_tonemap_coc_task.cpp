
#include "stdafx.h"
#include "hdr_tonemap_coc_task.h"

using namespace StE::Graphics;
using namespace StE::LLR;

void hdr_tonemap_coc_task::set_context_state() const {
	Base::set_context_state();
	
	0_storage_idx = p->histogram_sums;
	2_storage_idx = p->hdr_bokeh_param_buffer;
	ScreenFillingQuad.vao()->bind();
	
	p->fbo_hdr.bind();
	p->hdr_tonemap_coc->bind();
}

void hdr_tonemap_coc_task::dispatch() const {
	gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
 