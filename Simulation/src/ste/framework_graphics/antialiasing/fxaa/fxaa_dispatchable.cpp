
#include "stdafx.hpp"
#include "fxaa_dispatchable.hpp"

#include "Quad.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;

void fxaa_dispatchable::set_context_state() const {
	using namespace Core;

	ScreenFillingQuad.vao()->bind();

	program.get().bind();
}

void fxaa_dispatchable::dispatch() const {
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
