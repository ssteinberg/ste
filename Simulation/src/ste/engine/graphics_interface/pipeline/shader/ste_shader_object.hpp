// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_shader.hpp>
#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage.hpp>

#include <vector>
#include <string>

namespace StE {
namespace GL {

struct ste_shader_object {
	const vk_shader shader;

	const ste_shader_stage stage;
	const std::vector<ste_shader_stage_binding> stage_bindings;

	ste_shader_object() = delete;
	ste_shader_object(const vk_logical_device &device, 
					  const std::string &shader_code,
					  const ste_shader_stage &stage,
					  std::vector<ste_shader_stage_binding> &&stage_bindings)
		: shader(device,
				 shader_code),
		stage(stage),
		stage_bindings(std::move(stage_bindings))
	{
		assert(stage != ste_shader_stage::none);
	}

	/**
	*	@brief	Retrieve the Vulkan stage flag for the shader module
	*/
	VkShaderStageFlagBits vk_shader_stage_flag() const {
		switch (stage) {
		case ste_shader_stage::vertex_program:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ste_shader_stage::tesselation_control_program:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ste_shader_stage::tesselation_evaluation_program:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ste_shader_stage::geometry_program:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ste_shader_stage::fragment_program:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ste_shader_stage::compute_program:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			assert(false);
			return VK_SHADER_STAGE_ALL;
		}
	}
};

}
}
