
#include "stdafx.hpp"
#include "ssss_bilateral_blur_x.hpp"

#include "gl_current_context.hpp"
#include "Quad.hpp"

#include "Sampler.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void ssss_bilateral_blur_x::set_context_state() const {
	auto size = p->ssss->layers_size();

	Core::GL::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	7_tex_unit = *p->ssss->get_z_buffer();
	7_sampler_idx = *Sampler::SamplerNearestClamp();
	8_tex_unit = *p->ssss->get_penumbra_layers();
	8_sampler_idx = *Sampler::SamplerNearestClamp();

	ssss_blur_program.get().bind();

	ScreenFillingQuad.vao()->bind();
	p->ssss->get_blur_fbo()->bind();
}

void ssss_bilateral_blur_x::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	Core::GL::gl_current_context::get()->draw_arrays_instanced(GL_TRIANGLE_STRIP, 0, 4, p->scene->scene_properties().lights_storage().size());
}
