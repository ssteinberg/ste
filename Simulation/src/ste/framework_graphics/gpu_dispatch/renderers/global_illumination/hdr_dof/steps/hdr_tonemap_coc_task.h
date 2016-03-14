// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"
#include "hdr_compute_histogram_sums_task.h"

namespace StE {
namespace Graphics {

class hdr_tonemap_coc_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	hdr_tonemap_coc_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::add_dependency(std::make_shared<hdr_compute_histogram_sums_task>(p));
	}
	~hdr_tonemap_coc_task() noexcept {}

	void set_context_state() const override final {
		using namespace LLR;

		0_storage_idx = p->histogram_sums;
		2_storage_idx = p->hdr_bokeh_param_buffer;
		ScreenFillingQuad.vao()->bind();
		p->fbo_hdr.bind();
		p->hdr_tonemap_coc->bind();
	}
	
	void dispatch() const override final {
		gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
		gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
