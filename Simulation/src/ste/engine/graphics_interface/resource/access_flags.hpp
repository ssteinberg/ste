//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

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

}
}
