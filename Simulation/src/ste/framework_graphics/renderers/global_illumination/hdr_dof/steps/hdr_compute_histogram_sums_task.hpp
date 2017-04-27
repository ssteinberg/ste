// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <gl_current_context.hpp>

#include <memory>

namespace ste {
namespace Graphics {

class hdr_compute_histogram_sums_task : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	hdr_dof_postprocess *p;

public:
	hdr_compute_histogram_sums_task(hdr_dof_postprocess *p) : p(p) {}
	~hdr_compute_histogram_sums_task() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
