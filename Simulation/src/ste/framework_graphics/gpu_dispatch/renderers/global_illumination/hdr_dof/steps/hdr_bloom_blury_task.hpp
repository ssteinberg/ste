// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "gl_current_context.hpp"
#include "hdr_bloom_blurx_task.hpp"

namespace StE {
namespace Graphics {

class hdr_bloom_blury_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	hdr_bloom_blury_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::sub_tasks.insert(std::make_shared<hdr_bloom_blurx_task>(p));
	}
	~hdr_bloom_blury_task() noexcept {}

protected:
	void set_context_state() const override final {
		Base::set_context_state();
		
		ScreenFillingQuad.vao()->bind();
		p->fbo_hdr_final.bind();
		p->hdr_bloom_blury->bind();
	}
	
	void dispatch() const override final {
		Core::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
