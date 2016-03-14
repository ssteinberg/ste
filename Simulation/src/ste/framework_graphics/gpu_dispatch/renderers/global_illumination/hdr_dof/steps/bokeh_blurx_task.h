// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"
#include "hdr_bloom_blury_task.h"

namespace StE {
namespace Graphics {

class bokeh_blurx_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	bokeh_blurx_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::add_dependency(std::make_shared<hdr_bloom_blury_task>(p));
	}
	~bokeh_blurx_task() noexcept {}

	void set_context_state() const override final {
		ScreenFillingQuad.vao()->bind();
		p->fbo_bokeh_blur_image.bind();
		p->bokeh_blurx->bind();
	}
	
	void dispatch() const override final {
		gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
