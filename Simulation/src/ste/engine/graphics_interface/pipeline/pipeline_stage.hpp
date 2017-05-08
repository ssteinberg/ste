//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class pipeline_stage : std::uint32_t {
	top_of_pipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	draw_indirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	vertex_input = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	vertex_shader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	tessellation_control_shader = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
	tessellation_evaluation_shader = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
	geometry_shader = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
	fragment_shader = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	early_fragment_tests = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	late_fragment_tests = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
	color_attachment_output = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	compute_shader = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	transfer = VK_PIPELINE_STAGE_TRANSFER_BIT,
	bottom_of_pipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	host = VK_PIPELINE_STAGE_HOST_BIT,
	all_graphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
	all_commands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	command_process_nvx = VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX,
};

constexpr auto operator|(const pipeline_stage &lhs, const pipeline_stage &rhs) {
	using T = std::underlying_type_t<pipeline_stage>;
	return static_cast<pipeline_stage>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const pipeline_stage &lhs, const pipeline_stage &rhs) {
	using T = std::underlying_type_t<pipeline_stage>;
	return static_cast<pipeline_stage>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const pipeline_stage &lhs, const pipeline_stage &rhs) {
	using T = std::underlying_type_t<pipeline_stage>;
	return static_cast<pipeline_stage>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
