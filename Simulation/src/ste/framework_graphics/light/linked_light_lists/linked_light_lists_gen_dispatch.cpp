
#include "stdafx.hpp"
#include "linked_light_lists_gen_dispatch.hpp"

#include "Quad.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void linked_light_lists_gen_dispatch::set_context_state() const {
	GL::gl_current_context::get()->color_mask(false, false, false, false);
	GL::gl_current_context::get()->depth_mask(false);

	auto& size = lll->get_size();
	GL::gl_current_context::get()->viewport(0, 0, size.x, size.y);

	ls->bind_lights_buffer(2);
	4_storage_idx = Core::buffer_object_cast<Core::ShaderStorageBuffer<std::uint32_t>>(ls->get_active_ll_counter());
	5_storage_idx = ls->get_active_ll();

	lll->bind_lll_buffer(false);

	11_sampler_idx = depth_sampler;
	11_tex_unit = *depth_map;

	ScreenFillingQuad.vao()->bind();

	program.get().bind();
}

void linked_light_lists_gen_dispatch::dispatch() const {
	lll->clear();
	GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	GL::gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
}
