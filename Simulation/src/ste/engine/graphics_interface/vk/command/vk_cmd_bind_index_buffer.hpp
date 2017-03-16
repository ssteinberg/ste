//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

class vk_cmd_bind_index_buffer : public vk_command {
private:
	VkBuffer buffer;
	std::uint64_t offset;
	VkIndexType index_type;

public:
	template <bool sparse>
	vk_cmd_bind_index_buffer(const vk_buffer<std::uint32_t, sparse> &buffer,
							 std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT32)
	{}
	template <bool sparse>
	vk_cmd_bind_index_buffer(const vk_buffer<std::uint16_t, sparse> &buffer,
							 std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), index_type(VK_INDEX_TYPE_UINT16)
	{}
	virtual ~vk_cmd_bind_index_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBindIndexBuffer(command_buffer, buffer, offset, index_type);
	}
};

}
}
