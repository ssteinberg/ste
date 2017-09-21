//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class presentation_surface_composite_alpha : std::uint32_t {
	presentation_composite_alpha_opaque = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	presentation_composite_alpha_pre_multiplied = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
	presentation_composite_alpha_post_multiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
	presentation_composite_alpha_inherit = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

}
}
