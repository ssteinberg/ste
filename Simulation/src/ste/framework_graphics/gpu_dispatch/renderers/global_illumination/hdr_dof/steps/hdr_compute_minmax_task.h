// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"

namespace StE {
namespace Graphics {

class hdr_compute_minmax_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	hdr_compute_minmax_task(hdr_dof_postprocess *p) : p(p) {}
	~hdr_compute_minmax_task() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
