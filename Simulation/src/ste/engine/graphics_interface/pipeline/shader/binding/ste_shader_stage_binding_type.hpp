// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class ste_shader_stage_binding_type : std::uint16_t {
	unknown,
	uniform,
	storage,
	push_constant,
	spec_constant,
};

}
}
