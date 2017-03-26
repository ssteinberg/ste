//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class cmd_dispatch_indirect : public command {
private:
	VkBuffer buffer;
	std::uint32_t offset;

public:
	cmd_dispatch_indirect(const vk_buffer_base &buffer,
							 std::uint32_t offset = 0) : buffer(buffer), offset(offset * buffer.get_element_size_bytes())
	{}
	virtual ~cmd_dispatch_indirect() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDispatchIndirect(command_buffer, buffer, offset);
	}
};

}
}
