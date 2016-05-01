
#include "stdafx.hpp"
#include "scene_prepopulate_depth_dispatch.hpp"

#include "Scene.hpp"

using namespace StE::Graphics;

void scene_prepopulate_depth_dispatch::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);

	scene->object_group().bind_buffers();

	program->bind();
	scene->get_fbo()->bind();
}

void scene_prepopulate_depth_dispatch::dispatch() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
	scene->object_group().draw_object_group();
}
