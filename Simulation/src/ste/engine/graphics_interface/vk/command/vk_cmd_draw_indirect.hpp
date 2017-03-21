//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class vk_cmd_draw_indirect : public vk_command {
private:
	VkBuffer buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	template <typename T, bool sparse>
	vk_cmd_draw_indirect(const vk_buffer_base &buffer,
						 std::uint32_t offset,
						 std::uint32_t draw_count,
						 std::uint32_t stride)
		: buffer(buffer),
		offset(offset * buffer.get_element_size_bytes()),
		draw_count(draw_count),
		stride(stride * buffer.get_element_size_bytes())
	{}
	virtual ~vk_cmd_draw_indirect() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDrawIndirect(command_buffer, buffer, offset, draw_count, stride);
	}
};

}
}
