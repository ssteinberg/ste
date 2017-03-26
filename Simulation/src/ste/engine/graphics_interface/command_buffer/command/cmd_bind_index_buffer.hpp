//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

class cmd_bind_index_buffer : public command {
private:
	VkBuffer buffer;
	std::uint64_t offset;
	VkIndexType index_type;

public:
	cmd_bind_index_buffer(const vk_buffer<std::uint32_t, false> &buffer,
						  std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT32)
	{}
	cmd_bind_index_buffer(const vk_buffer<std::uint32_t, true> &buffer,
						  std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT32)
	{}
	cmd_bind_index_buffer(const vk_buffer<std::uint16_t, false> &buffer,
						  std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT16)
	{}
	cmd_bind_index_buffer(const vk_buffer<std::uint16_t, true> &buffer,
						  std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT16)
	{}
	virtual ~cmd_bind_index_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdBindIndexBuffer(command_buffer, buffer, offset, index_type);
	}
};

}
}
