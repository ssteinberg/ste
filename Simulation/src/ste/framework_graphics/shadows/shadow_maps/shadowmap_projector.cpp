
#include "stdafx.hpp"
#include "shadowmap_projector.hpp"

using namespace StE::Graphics;

void shadowmap_projector::set_context_state() const {
	using namespace Core;

	GL::gl_current_context::get()->enable_depth_test();
	GL::gl_current_context::get()->color_mask(false, false, false, false);
	GL::gl_current_context::get()->enable_state(GL::BasicStateName::CULL_FACE);
	GL::gl_current_context::get()->cull_face(GL_FRONT);

	auto size = shadow_map->get_cubemaps()->get_size();
	GL::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	object->bind_buffers();
	lights->bind_buffers(2);

	5_storage_idx = *shadow_map->get_shadow_projection_mats_buffer();

	shadow_gen_program->bind();

	shadow_map->get_fbo()->bind();
}

void shadowmap_projector::dispatch() const {
	Core::GL::gl_current_context::get()->clear_framebuffer(false, true);

	lights->update_storage();

	object->dispatch();

	lights->lock_ranges();
}
