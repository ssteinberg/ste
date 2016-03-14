// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"

namespace StE {
namespace Graphics {

class hdr_compute_minmax_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	hdr_compute_minmax_task(hdr_dof_postprocess *p) : p(p) {}
	~hdr_compute_minmax_task() noexcept {}

	void set_context_state() const override final {
		using namespace LLR;
	
		0_image_idx = (*p->hdr_lums)[0];
		2_storage_idx = p->hdr_bokeh_param_buffer;
		3_storage_idx = p->hdr_bokeh_param_buffer_prev;
		p->hdr_compute_minmax->bind();
	}
	
	void dispatch() const override final {
		gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
		p->hdr_compute_minmax->set_uniform("time", p->ctx.time_per_frame().count());
		
		p->hdr_bokeh_param_buffer_prev << p->hdr_bokeh_param_buffer;
		p->hdr_bokeh_param_buffer << *p->hdr_bokeh_param_buffer_eraser;
		
		gl_current_context::get()->dispatch_compute(p->luminance_size.x / 32, 
													p->luminance_size .y / 32, 
													1);
	}
};

}
}
