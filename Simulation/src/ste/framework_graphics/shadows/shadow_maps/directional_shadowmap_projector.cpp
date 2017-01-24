
#include "stdafx.hpp"
#include "directional_shadowmap_projector.hpp"

using namespace StE::Graphics;

void directional_shadowmap_projector::set_context_state() const {
	using namespace Core;

	GL::gl_current_context::get()->enable_depth_test();
	GL::gl_current_context::get()->color_mask(false, false, false, false);
	GL::gl_current_context::get()->enable_state(GL::BasicStateName::CULL_FACE);
	//GL::gl_current_context::get()->cull_face(GL_FRONT);
	
	auto size = shadow_map->get_directional_maps()->get_size();
	GL::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	s->get_directional_shadow_projection_buffers().idb.buffer().bind();
	s->bind_buffers();
	lights->bind_lights_buffer(2);

	4_storage_idx = Core::buffer_object_cast<Core::shader_storage_buffer<std::uint32_t>>(lights->get_active_ll_counter());
	5_storage_idx = lights->get_active_ll();
	6_storage_idx = lights->get_directional_lights_cascades_buffer();

	8_storage_idx = s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table;

	shadow_gen_program.get().bind();
	shadow_map->get_directional_maps_fbo()->bind();
}

void directional_shadowmap_projector::dispatch() const {
	Core::GL::gl_current_context::get()->clear_framebuffer(false, true);

	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	s->draw_object_group();
}
