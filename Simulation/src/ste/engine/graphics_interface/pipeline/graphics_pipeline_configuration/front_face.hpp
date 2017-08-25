//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class front_face : std::uint32_t {
	ccw = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	cw = VK_FRONT_FACE_CLOCKWISE,
};

}
}
