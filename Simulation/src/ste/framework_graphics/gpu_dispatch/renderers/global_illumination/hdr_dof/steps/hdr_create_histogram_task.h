// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"
#include "hdr_compute_minmax_task.h"

namespace StE {
namespace Graphics {

class hdr_create_histogram_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	hdr_create_histogram_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::add_dependency(std::make_shared<hdr_compute_minmax_task>(p));
	}
	~hdr_create_histogram_task() noexcept {}

	void set_context_state() const override final {
		using namespace LLR;

		0_atomic_idx = p->histogram;
		0_image_idx = (*p->hdr_lums)[0];
		2_storage_idx = p->hdr_bokeh_param_buffer;
		p->hdr_create_histogram->bind();
	}
	
	void dispatch() const override final {
		std::uint32_t zero = 0;
		p->histogram.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
		
		gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		gl_current_context::get()->dispatch_compute(p->luminance_size.x / 32, 
													p->luminance_size .y / 32, 
													1);
	}
};

}
}
