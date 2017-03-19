//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>

namespace StE {
namespace GL {

class vk_cmd_update_buffer : public vk_command {
private:
	VkBuffer buffer;
	std::uint64_t offset{ 0 };
	std::uint64_t size;
	const void* data;

public:
	template <typename T, bool sparse>
	vk_cmd_update_buffer(const vk_buffer<T, sparse> &buffer,
						 std::uint64_t data_size,
						 const void *data,
						 std::uint64_t offset = 0) 
		: buffer(buffer), offset(offset * sizeof(T)), size(data_size * sizeof(T)), data(data)
	{}
	virtual ~vk_cmd_update_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdUpdateBuffer(command_buffer, buffer, offset, size, data);
	}
};

}
}
