//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <cassert>

namespace StE {
namespace GL {

class vk_cmd_bind_index_buffer : public vk_command {
private:
	VkBuffer buffer;
	std::uint64_t offset;

public:
	vk_cmd_bind_index_buffer(VkBuffer buffer, std::uint64_t offset) : buffer(buffer), offset(offset)
	{}
	virtual ~vk_cmd_bind_index_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBindIndexBuffer(command_buffer, buffer, offset, VK_INDEX_TYPE_UINT32);
	}
};

}
}
