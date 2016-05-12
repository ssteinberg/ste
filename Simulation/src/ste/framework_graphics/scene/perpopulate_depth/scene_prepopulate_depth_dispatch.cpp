
#include "stdafx.hpp"
#include "scene_prepopulate_depth_dispatch.hpp"

#include "Scene.hpp"

using namespace StE::Graphics;

void scene_prepopulate_depth_dispatch::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);

	scene->get_idb().buffer().bind();
	scene->bind_buffers();

	program->bind();
}

void scene_prepopulate_depth_dispatch::dispatch() const {
	scene->draw_object_group();
}
