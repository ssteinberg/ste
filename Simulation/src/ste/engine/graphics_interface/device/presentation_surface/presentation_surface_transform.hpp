//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class presentation_surface_transform : std::uint32_t {
	presentation_surface_transform_identity = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	presentation_surface_transform_rotate_90 = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
	presentation_surface_transform_rotate_180 = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
	presentation_surface_transform_rotate_270 = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
	presentation_surface_transform_horizontal_mirror = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
	presentation_surface_transform_horizontal_mirror_rotate_90 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
	presentation_surface_transform_horizontal_mirror_rotate_180 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
	presentation_surface_transform_horizontal_mirror_rotate_270 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
	presentation_surface_transform_inherit = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
};

}
}
