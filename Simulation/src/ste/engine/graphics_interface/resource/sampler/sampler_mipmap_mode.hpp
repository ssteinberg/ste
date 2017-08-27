//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class sampler_mipmap_mode : std::uint32_t {
	nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

}
}
