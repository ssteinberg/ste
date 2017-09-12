//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>

namespace ste {
namespace gl {

class cmd_draw_indirect : public command {
private:
	VkBuffer buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	cmd_draw_indirect(cmd_draw_indirect &&) = default;
	cmd_draw_indirect(const cmd_draw_indirect&) = default;
	cmd_draw_indirect &operator=(cmd_draw_indirect &&) = default;
	cmd_draw_indirect &operator=(const cmd_draw_indirect&) = default;

	cmd_draw_indirect(const device_buffer_base &buffer,
					  std::uint32_t offset,
					  std::uint32_t draw_count,
					  std::uint32_t stride = 1)
		: buffer(buffer.get_buffer_handle()),
		  offset(offset * buffer.get_element_size_bytes()),
		  draw_count(draw_count),
		  stride(stride * buffer.get_element_size_bytes()) {}

	virtual ~cmd_draw_indirect() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDrawIndirect(command_buffer, buffer, offset, draw_count, stride);
	}
};

}
}
