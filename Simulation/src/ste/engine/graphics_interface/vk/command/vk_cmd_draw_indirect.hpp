//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

template <typename T>
class vk_cmd_draw_indirect : public vk_command {
private:
	const vk_buffer<T> &buffer;
	std::uint32_t offset;
	std::uint32_t draw_count;
	std::uint32_t stride;

public:
	vk_cmd_draw_indirect(const vk_buffer<T> &buffer,
						 std::uint32_t offset,
						 std::uint32_t draw_count,
						 std::uint32_t stride = sizeof(T))
		: buffer(buffer),
		offset(offset),
		draw_count(draw_count),
		stride(stride)
	{}
	virtual ~vk_cmd_draw_indirect() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDrawIndirect(command_buffer, buffer, offset, draw_count, stride);
	}
};

}
}
