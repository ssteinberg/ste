//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

class vk_cmd_dispatch : public vk_command {
private:
	VkBuffer buffer;
	std::uint32_t offset;

public:
	template <typename T>
	vk_cmd_dispatch(const vk_buffer<T> &buffer,
					std::uint32_t offset = 0) : buffer(buffer), offset(offset)
	{}
	virtual ~vk_cmd_dispatch() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDispatchIndirect(command_buffer, buffer, offset);
	}
};

}
}
