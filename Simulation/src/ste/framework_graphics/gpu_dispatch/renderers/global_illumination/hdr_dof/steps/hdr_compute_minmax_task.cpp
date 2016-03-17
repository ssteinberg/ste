
#include "stdafx.hpp"
#include "hdr_compute_minmax_task.hpp"

using namespace StE::Graphics;
using namespace StE::LLR;

void hdr_compute_minmax_task::set_context_state() const {
	Base::set_context_state();
	
	0_image_idx = (*p->hdr_lums)[0];
	2_storage_idx = p->hdr_bokeh_param_buffer;
	3_storage_idx = p->hdr_bokeh_param_buffer_prev;
	
	p->hdr_compute_minmax->bind();
}

void hdr_compute_minmax_task::dispatch() const {
	gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	p->hdr_compute_minmax->set_uniform("time", p->ctx.time_per_frame().count());
	
	p->hdr_bokeh_param_buffer_prev << p->hdr_bokeh_param_buffer;
	p->hdr_bokeh_param_buffer << *p->hdr_bokeh_param_buffer_eraser;
	
	gl_current_context::get()->dispatch_compute(p->luminance_size.x / 32, 
												p->luminance_size .y / 32, 
												1);
}
 