//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class image_type {
	image_1d,
	image_1d_array,
	image_2d,
	image_2d_array,
	image_3d,
	image_cubemap,
	image_cubemap_array,
};

}
}
