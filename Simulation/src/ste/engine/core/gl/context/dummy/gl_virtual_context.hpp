// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gl_generic_context.hpp"

namespace StE {
namespace Core {

class gl_virtual_context : public gl_generic_context {
	using Base = gl_generic_context;
	
public:
	gl_virtual_context() : Base(true) {}

	std::size_t get_total_state_changes() const { return total_state_changes; };
	std::size_t get_total_buffer_changes() const { return total_buffer_changes; };
	std::size_t get_total_texture_changes() const { return total_texture_changes; };
	std::size_t get_total_shader_changes() const { return total_shader_changes; };
	std::size_t get_total_fbo_changes() const { return total_fbo_changes; };
	std::size_t get_total_va_changes() const { return total_va_changes; };
	
	const auto &get_states() const { return Base::states; }
	const auto &get_resources() const { return Base::resources; }
};

}
}
