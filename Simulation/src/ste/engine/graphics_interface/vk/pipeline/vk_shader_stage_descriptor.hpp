//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_shader.hpp>

namespace StE {
namespace GL {

struct vk_shader_stage_descriptor {
	const vk_shader *shader;
	const vk_shader::spec_map *specializations{ nullptr };
	VkShaderStageFlagBits stage;
};

}
}
