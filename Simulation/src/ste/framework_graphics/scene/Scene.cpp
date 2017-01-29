
#include "stdafx.hpp"
#include "scene.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program.hpp"

#include "gl_current_context.hpp"

#include "object_group_draw_buffers.hpp"

using namespace StE::Graphics;

constexpr int scene::shadow_pltt_size;
constexpr int scene::directional_shadow_pltt_size;

scene::scene(const ste_engine_control &ctx) : culled_objects_counter(1),
										   object_program(ctx, std::vector<std::string>{ "scene_transform.vert", "object.frag" }) {}

void scene::bind_buffers() const {
	using namespace Core;

	13_storage_idx = scene_props.materials_storage().buffer();
	objects.get_draw_buffers().bind_buffers(14);
}

void scene::set_context_state() const {
	Core::GL::gl_current_context::get()->enable_depth_test();
	Core::GL::gl_current_context::get()->depth_func(GL_EQUAL);
	Core::GL::gl_current_context::get()->color_mask(false, false, false, false);
	Core::GL::gl_current_context::get()->depth_mask(false);

	Core::GL::gl_current_context::get()->enable_state(Core::GL::BasicStateName::CULL_FACE);

	gbuffer->bind_gbuffer();
	idb.buffer().bind();
	bind_buffers();

	object_program.get().bind();
}

void scene::draw_object_group() const {
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
	Core::GL::gl_current_context::get()->draw_multi_elements_indirect<object_group_draw_buffers::elements_type::T>(GL_TRIANGLES, 
																												   nullptr, 
																												   objects.get_draw_buffers().size(), 
																												   0);
}

void scene::dispatch() const {
	draw_object_group();
}
