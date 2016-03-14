// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"
#include "hdr_create_histogram_task.h"

namespace StE {
namespace Graphics {

class hdr_compute_histogram_sums_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	hdr_compute_histogram_sums_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::add_dependency(std::make_shared<hdr_create_histogram_task>(p));
	}
	~hdr_compute_histogram_sums_task() noexcept {}

	void set_context_state() const override final {
		using namespace LLR;
	
		0_storage_idx = p->histogram_sums;
		1_storage_idx = p->histogram;
		2_storage_idx = p->hdr_bokeh_param_buffer;
		p->hdr_compute_histogram_sums->bind();
	}
	
	void dispatch() const override final {
		gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
		p->hdr_compute_histogram_sums->set_uniform("time", p->ctx.time_per_frame().count());
		
		gl_current_context::get()->dispatch_compute(1, 1, 1);
	}
};

}
}
