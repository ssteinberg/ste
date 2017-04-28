// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_shader.hpp>

#include <ste_shader_program_stage.hpp>
#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage_attachment.hpp>

#include <vector>
#include <string>

namespace ste {
namespace gl {

struct ste_shader_object {
	const vk::vk_shader shader;

	const ste_shader_program_stage stage;
	const std::vector<ste_shader_stage_binding> stage_bindings;
	const std::vector<ste_shader_stage_attachment> stage_attachments;

	ste_shader_object() = delete;
	ste_shader_object(const vk::vk_logical_device &device,
					  const std::string &shader_code,
					  const ste_shader_program_stage &stage,
					  std::vector<ste_shader_stage_binding> &&stage_bindings,
					  std::vector<ste_shader_stage_attachment> &&stage_attachments = {})
		: shader(device,
				 shader_code),
		stage(stage),
		stage_bindings(std::move(stage_bindings)),
		stage_attachments(std::move(stage_attachments))
	{
		assert(stage != ste_shader_program_stage::none);
	}

	/**
	*	@brief	Retrieve the stage flag for the shader module
	*/
	stage_flag shader_stage_flag() const {
		return ste_shader_program_stage_to_stage_flag(stage);
	}
};

}
}
