//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class blend_op : std::uint32_t {
	add = VK_BLEND_OP_ADD,
	subtract = VK_BLEND_OP_SUBTRACT,
	reverse_subtract = VK_BLEND_OP_REVERSE_SUBTRACT,
	min = VK_BLEND_OP_MIN,
	max = VK_BLEND_OP_MAX,
};

}
}
