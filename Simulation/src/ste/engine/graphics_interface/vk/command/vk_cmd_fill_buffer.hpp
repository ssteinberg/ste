//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class vk_cmd_fill_buffer : public vk_command {
private:
	VkBuffer buffer;
	std::uint64_t offset{ 0 };
	std::uint64_t size{ VK_WHOLE_SIZE };
	std::uint32_t data;

public:
	vk_cmd_fill_buffer(const vk_buffer_base &buffer,
					   std::uint32_t data) : buffer(buffer), data(data)
	{}
	vk_cmd_fill_buffer(const vk_buffer_base &buffer,
					   float data) : buffer(buffer), data(*reinterpret_cast<std::uint32_t*>(&data))
	{}
	vk_cmd_fill_buffer(const vk_buffer_base &buffer,
					   std::uint32_t data,
					   std::uint64_t size,
					   std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), size(size), data(data)
	{}
	vk_cmd_fill_buffer(const vk_buffer_base &buffer,
					   float data,
					   std::uint64_t size,
					   std::uint64_t offset = 0)
		: buffer(buffer), offset(offset), size(size), data(*reinterpret_cast<std::uint32_t*>(&data))
	{}
	virtual ~vk_cmd_fill_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdFillBuffer(command_buffer, buffer, offset, size, data);
	}
};

}
}
