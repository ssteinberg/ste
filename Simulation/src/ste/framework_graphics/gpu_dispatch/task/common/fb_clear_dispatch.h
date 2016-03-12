// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_task.h"

#include "gl_current_context.h"

namespace StE {
namespace Graphics {

template <bool color = true, bool depth = true>
class fb_clear_dispatch : public gpu_task {
protected:
	virtual void dispatch() const override {
		gl_current_context::get()->clear_framebuffer(color, depth);
	}
};
	
}
}
