// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "gl_current_context.hpp"
#include "hdr_compute_histogram_sums_task.hpp"

namespace StE {
namespace Graphics {

class hdr_tonemap_coc_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	hdr_tonemap_coc_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::sub_tasks.insert(std::make_shared<hdr_compute_histogram_sums_task>(p));
	}
	~hdr_tonemap_coc_task() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
