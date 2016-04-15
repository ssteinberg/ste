
#include "stdafx.hpp"
#include "ssss_bilateral_blur_y.hpp"

#include "gl_current_context.hpp"
#include "Quad.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void ssss_bilateral_blur_y::set_context_state() const {
	auto size = p->ssss->layers_size();

	Core::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	p->scene->scene_properties().lights_storage().bind_buffers(2);
	p->deferred->bind_output_textures();

	7_tex_unit = *p->ssss->get_blur_layers();

	ssss_blur_program->bind();

	ScreenFillingQuad.vao()->bind();
	p->ssss->get_penumbra_fbo()->bind();
}

void ssss_bilateral_blur_y::dispatch() const {
	Core::gl_current_context::get()->draw_arrays_instanced(GL_TRIANGLE_STRIP, 0, 4, p->scene->scene_properties().lights_storage().get_lights().size());
}
