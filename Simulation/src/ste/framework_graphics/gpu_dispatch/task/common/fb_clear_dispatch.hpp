// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"

#include "gl_current_context.hpp"

namespace StE {
namespace Graphics {

template <bool color = true, bool depth = true>
class fb_clear_dispatch : public gpu_task {
	using Base = gpu_task;
	
protected:
	void set_context_state() const override {
		Base::set_context_state();
	}
	
	virtual void dispatch() const override {
		Core::gl_current_context::get()->clear_framebuffer(color, depth);
	}
};
	
}
}
