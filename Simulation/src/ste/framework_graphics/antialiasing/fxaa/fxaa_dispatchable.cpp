
#include "stdafx.hpp"
#include "fxaa_dispatchable.hpp"

#include "Sampler.hpp"
#include "Quad.hpp"

#include "gl_current_context.hpp"

using namespace StE::Graphics;

void fxaa_dispatchable::set_context_state() const {
	using namespace Core;

	0_tex_unit = *input;
	0_sampler_idx = *Sampler::SamplerLinearClamp();

	ScreenFillingQuad.vao()->bind();

	program.get().bind();
}

void fxaa_dispatchable::dispatch() const {
	Core::GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
