
#include "stdafx.hpp"
#include "ssss_generator.hpp"

using namespace StE::Graphics;

void ssss_generator::set_context_state() const {
	using namespace Core;

	deferred->bind_output_textures();
	0_image_idx = ssss->get_penumbra_layers()->make_image(0);
	8_tex_unit = *scene->shadow_storage().get_cubemaps();

	scene->scene_properties().lights_storage().bind_buffers(2);

	ssss_gen_program->bind();

	// ScreenFillingQuad.vao()->bind();
}

void ssss_generator::dispatch() const {
	// Core::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	auto size = ssss->layers_size();

	Core::gl_current_context::get()->dispatch_compute(size.x / 32, size.y / 32, 1);
}
