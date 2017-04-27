//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>

namespace ste {
namespace gl {

class cmd_draw_indexed_indirect : public command {
private:
	std::reference_wrapper<const vk::vk_buffer> buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	cmd_draw_indexed_indirect(const vk::vk_buffer &buffer,
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
		vkCmdDrawIndexedIndirect(command_buffer, buffer.get(), offset, draw_count, stride);
	}
};

}
}
