
#include <stdafx.hpp>
#include <scene_prepopulate_depth_dispatch.hpp>

#include <gl_current_context.hpp>

#include <Scene.hpp>

using namespace StE::Graphics;

void scene_prepopulate_depth_dispatch::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);
	if (!front_face)
		Core::GL::gl_current_context::get()->cull_face(GL_FRONT);

	s->get_idb().buffer().bind();
	s->bind_buffers();

	program.get().bind();
}

void scene_prepopulate_depth_dispatch::dispatch() const {
	if (front_face)
		Core::GL::gl_current_context::get()->clear_framebuffer(false, true);
	s->draw_object_group();
}
