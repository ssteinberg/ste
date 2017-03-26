//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class cmd_draw_indexed_indirect : public command {
private:
	VkBuffer buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	cmd_draw_indexed_indirect(const vk_buffer_base &buffer,
							  std::uint32_t offset,
							  std::uint32_t draw_count,
							  std::uint32_t stride)
		: buffer(buffer),
		offset(offset * buffer.get_element_size_bytes()),
		draw_count(draw_count),
		stride(stride * buffer.get_element_size_bytes())
	{}
	virtual ~cmd_draw_indexed_indirect() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDrawIndexedIndirect(command_buffer, buffer, offset, draw_count, stride);
	}
};

}
}
