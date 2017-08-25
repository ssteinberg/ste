//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>

#include <vk_shader.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
struct vk_shader_stage_descriptor {
	const vk_shader<host_allocator> *shader;
	const typename vk_shader<host_allocator>::spec_map *specializations{ nullptr };
	VkShaderStageFlagBits stage;
};

}

}
}
