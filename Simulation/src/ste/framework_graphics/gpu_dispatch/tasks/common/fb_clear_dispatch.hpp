// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dispatchable.hpp"

#include "gl_current_context.hpp"

namespace StE {
namespace Graphics {

template <bool color = true, bool depth = true>
class fb_clear_dispatch : public gpu_dispatchable {
	using Base = gpu_dispatchable;

protected:
	virtual void set_context_state() const override {}

	virtual void dispatch() const override {
		Core::GL::gl_current_context::get()->clear_framebuffer(color, depth);
	}
};

}
}
