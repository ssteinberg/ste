// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <hdr_dof_postprocess.hpp>

#include <gl_current_context.hpp>

#include <memory>

namespace ste {
namespace Graphics {

class hdr_bloom_blury_task : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	hdr_dof_postprocess *p;

public:
	hdr_bloom_blury_task(hdr_dof_postprocess *p) : p(p) {}
	~hdr_bloom_blury_task() noexcept {}

protected:
	void set_context_state() const override final {
		// TODO: Fix
//		screen_filling_quad.vao()->bind();
		p->hdr_bloom_blury.get().bind();
	}

	void dispatch() const override final {
		Core::gl::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
