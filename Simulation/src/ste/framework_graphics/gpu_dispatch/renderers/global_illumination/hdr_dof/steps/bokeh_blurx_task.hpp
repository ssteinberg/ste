// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "gl_current_context.hpp"
#include "hdr_bloom_blury_task.hpp"

namespace StE {
namespace Graphics {

class bokeh_blurx_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	bokeh_blurx_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::sub_tasks.insert(std::make_shared<hdr_bloom_blury_task>(p));
	}
	~bokeh_blurx_task() noexcept {}

protected:
	void set_context_state() const override final {
		Base::set_context_state();
		
		ScreenFillingQuad.vao()->bind();
		p->fbo_bokeh_blur_image.bind();
		p->bokeh_blurx->bind();
	}
	
	void dispatch() const override final {
		LLR::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
