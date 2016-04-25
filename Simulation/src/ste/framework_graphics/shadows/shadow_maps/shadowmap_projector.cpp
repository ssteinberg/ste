
#include "stdafx.hpp"
#include "shadowmap_projector.hpp"

using namespace StE::Graphics;

void shadowmap_projector::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);
	Core::GL::gl_current_context::get()->cull_face(GL_FRONT);

	object->bind_buffers();

	auto size = shadow_map->get_cubemaps()->get_size();
	Core::GL::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	lights->bind_buffers(2);
	shadow_gen_program->bind();
}

void shadowmap_projector::dispatch() const {
	Core::GL::gl_current_context::get()->clear_framebuffer(false, true);

	lights->update_storage();

	object->dispatch();

	lights->lock_ranges();
}
