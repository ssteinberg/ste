//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_stage.hpp>

namespace ste {
namespace gl {

enum class access_flags : std::uint32_t {
	none = 0,
	indirect_command_read = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
	index_read = VK_ACCESS_INDEX_READ_BIT,
	vertex_attribute_read = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
	uniform_read = VK_ACCESS_UNIFORM_READ_BIT,
	input_attachment_read = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
	shader_read = VK_ACCESS_SHADER_READ_BIT,
	shader_write = VK_ACCESS_SHADER_WRITE_BIT,
	color_attachment_read = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
	color_attachment_read_noncoherent = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
	color_attachment_write = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	depth_stencil_attachment_read = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
	depth_stencil_attachment_write = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	transfer_read = VK_ACCESS_TRANSFER_READ_BIT,
	transfer_write = VK_ACCESS_TRANSFER_WRITE_BIT,
	host_read = VK_ACCESS_HOST_READ_BIT,
	host_write = VK_ACCESS_HOST_WRITE_BIT,
	memory_read = VK_ACCESS_MEMORY_READ_BIT,
	memory_write = VK_ACCESS_MEMORY_WRITE_BIT,
	command_process_read_nvx = VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX,
	command_process_write_nvx = VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX,
};

constexpr auto operator|(const access_flags &lhs, const access_flags &rhs) {
	using T = std::underlying_type_t<access_flags>;
	return static_cast<access_flags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const access_flags &lhs, const access_flags &rhs) {
	using T = std::underlying_type_t<access_flags>;
	return static_cast<access_flags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const access_flags &lhs, const access_flags &rhs) {
	using T = std::underlying_type_t<access_flags>;
	return static_cast<access_flags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

/**
*	@brief	Returns the pipeline stage mask corresponding to the provided access flags.
*
*	@throws	ste_engine_exception	If no interpretation or more than a single interpretation is possible, e.g. uniform read access.
*/
auto inline pipeline_stage_for_access_flags(access_flags access) {
	pipeline_stage stage = static_cast<pipeline_stage>(0);

	if ((access & access_flags::indirect_command_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::draw_indirect;
	if ((access & access_flags::index_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::vertex_input;
	if ((access & access_flags::vertex_attribute_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::vertex_input;
	if ((access & access_flags::input_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::fragment_shader;
	if ((access & access_flags::color_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::color_attachment_read_noncoherent) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::color_attachment_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::depth_stencil_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::early_fragment_tests | pipeline_stage::late_fragment_tests;
	if ((access & access_flags::depth_stencil_attachment_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::early_fragment_tests | pipeline_stage::late_fragment_tests;
	if ((access & access_flags::transfer_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::transfer;
	if ((access & access_flags::transfer_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::transfer;
	if ((access & access_flags::host_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::host;
	if ((access & access_flags::host_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::host;
	if ((access & access_flags::command_process_read_nvx) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::command_process_nvx;
	if ((access & access_flags::command_process_write_nvx) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::command_process_nvx;

	if (stage == static_cast<pipeline_stage>(0)) {
		throw ste_engine_exception("Can not deduce pipeline stage by access flags. Access flags might be ambiguous.");
	}

	return stage;
}

/**
*	@brief	Returns mask of all the pipeline stages corresponding to the provided access flags. Doesn't throw.
*/
auto inline all_possible_pipeline_stages_for_access_flags(access_flags access) {
	pipeline_stage stage = static_cast<pipeline_stage>(0);
	constexpr pipeline_stage programmable_stages = pipeline_stage::vertex_shader | 
		pipeline_stage::tessellation_control_shader | 
		pipeline_stage::tessellation_evaluation_shader | 
		pipeline_stage::geometry_shader |
		pipeline_stage::fragment_shader |
		pipeline_stage::compute_shader;

	if ((access & access_flags::indirect_command_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::draw_indirect;
	if ((access & access_flags::index_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::vertex_input;
	if ((access & access_flags::vertex_attribute_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::vertex_input;
	if ((access & access_flags::uniform_read) != static_cast<access_flags>(0))
		stage = stage | programmable_stages;
	if ((access & access_flags::input_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::fragment_shader;
	if ((access & access_flags::shader_read) != static_cast<access_flags>(0))
		stage = stage | programmable_stages;
	if ((access & access_flags::shader_write) != static_cast<access_flags>(0))
		stage = stage | programmable_stages;
	if ((access & access_flags::color_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::color_attachment_read_noncoherent) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::color_attachment_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::color_attachment_output;
	if ((access & access_flags::depth_stencil_attachment_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::early_fragment_tests | pipeline_stage::late_fragment_tests;
	if ((access & access_flags::depth_stencil_attachment_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::early_fragment_tests | pipeline_stage::late_fragment_tests;
	if ((access & access_flags::transfer_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::transfer;
	if ((access & access_flags::transfer_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::transfer;
	if ((access & access_flags::host_read) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::host;
	if ((access & access_flags::host_write) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::host;
	if ((access & access_flags::command_process_read_nvx) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::command_process_nvx;
	if ((access & access_flags::command_process_write_nvx) != static_cast<access_flags>(0))
		stage = stage | pipeline_stage::command_process_nvx;

	return stage;
}

}
}
