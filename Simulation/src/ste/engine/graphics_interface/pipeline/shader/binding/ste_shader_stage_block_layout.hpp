// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

enum class ste_shader_stage_block_layout : std::uint16_t {
	none,
	std140,
	std430,
};

}
}
