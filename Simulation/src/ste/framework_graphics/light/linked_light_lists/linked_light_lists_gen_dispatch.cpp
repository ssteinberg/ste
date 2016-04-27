
#include "stdafx.hpp"
#include "linked_light_lists_gen_dispatch.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void linked_light_lists_gen_dispatch::set_context_state() const {
	ls->bind_lights_buffer(2);
	ls->bind_lights_transform_buffer(3);
	4_storage_idx = Core::buffer_object_cast<Core::ShaderStorageBuffer<std::uint32_t>>(ls->get_active_ll_counter());
	5_storage_idx = ls->get_active_ll();

	lll->bind_lll_buffer(false);

	11_tex_unit = *depth_map;

	program->bind();
}

void linked_light_lists_gen_dispatch::dispatch() const {
	const glm::ivec3 jobs = {16, 16, 1};

	auto global_invocations = glm::ivec3{ lll->get_size().x,
										  lll->get_size().y,
										  glm::min<int>(max_active_lights_per_frame, ls->size()) };
	auto size = (global_invocations + jobs - glm::ivec3(1)) / jobs;

	lll->clear();
	Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
	Core::GL::gl_current_context::get()->dispatch_compute(size.x, size.y, size.z);
}
