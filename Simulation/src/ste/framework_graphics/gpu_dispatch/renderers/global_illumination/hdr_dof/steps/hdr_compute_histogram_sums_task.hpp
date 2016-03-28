// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "gl_current_context.hpp"
#include "hdr_create_histogram_task.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class hdr_compute_histogram_sums_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	hdr_compute_histogram_sums_task(hdr_dof_postprocess *p) : p(p), gpu_task(std::make_unique<hdr_create_histogram_task>(p)) {}
	~hdr_compute_histogram_sums_task() noexcept {}

protected:
	void set_context_state() const override final;	
	void dispatch() const override final;
};

}
}
