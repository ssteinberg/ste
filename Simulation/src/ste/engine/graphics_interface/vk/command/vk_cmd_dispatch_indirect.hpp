//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

class vk_cmd_dispatch_indirect : public vk_command {
private:
	VkBuffer buffer;
	std::uint32_t offset;

public:
	template <typename T, bool sparse>
	vk_cmd_dispatch_indirect(const vk_buffer<T, sparse> &buffer,
							 std::uint32_t offset = 0) : buffer(buffer), offset(offset * sizeof(T))
	{}
	virtual ~vk_cmd_dispatch_indirect() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDispatchIndirect(command_buffer, buffer, offset);
	}
};

}
}
