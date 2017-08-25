// TODO
#include <stdafx.hpp>
//#include <linked_light_lists_gen_dispatch.hpp>
//
//using namespace ste::graphics;
//using namespace ste::Core;
//
//void linked_light_lists_gen_dispatch::set_context_state() const {
//	ls->bind_lights_buffer(2);
//	4_storage_idx = Core::buffer_object_cast<Core::shader_storage_buffer<std::uint32_t>>(ls->get_active_ll_counter());
//	5_storage_idx = ls->get_active_ll();
//
//	lll->bind_readwrite_lll_buffers();
//
//	program.get().bind();
//}
//
//void linked_light_lists_gen_dispatch::dispatch() const {
//	static const glm::ivec2 jobs = { 32, 32 };
//
//	auto size = (glm::ivec2{ lll->get_size().x, lll->get_size().y } +jobs - glm::ivec2(1)) / jobs;
//
//	lll->clear();
//
//	gl::gl_current_context::get()->memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
//	gl::gl_current_context::get()->dispatch_compute(size.x, size.y, 1);
//}
