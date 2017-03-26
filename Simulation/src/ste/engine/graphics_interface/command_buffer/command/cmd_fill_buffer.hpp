//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer_base.hpp>

namespace StE {
namespace GL {

class cmd_fill_buffer : public command {
private:
	VkBuffer buffer;
	std::uint64_t offset{ 0 };
	std::uint64_t size{ VK_WHOLE_SIZE };
	std::uint32_t data;

public:
	cmd_fill_buffer(const vk_buffer_base &buffer,
					std::uint32_t data) : buffer(buffer), data(data)
	{}
	cmd_fill_buffer(const vk_buffer_base &buffer,
					float data) : buffer(buffer), data(*reinterpret_cast<std::uint32_t*>(&data))
	{}
	cmd_fill_buffer(const vk_buffer_base &buffer,
					std::uint32_t data,
					std::uint64_t size,
					std::uint64_t offset = 0)
		: buffer(buffer),
		offset(offset * buffer.get_element_size_bytes()),
		size(size * buffer.get_element_size_bytes()),
		data(data)
	{}
	cmd_fill_buffer(const vk_buffer_base &buffer,
					float data,
					std::uint64_t size,
					std::uint64_t offset = 0)
		: buffer(buffer),
		offset(offset * buffer.get_element_size_bytes()),
		size(size * buffer.get_element_size_bytes()),
		data(*reinterpret_cast<std::uint32_t*>(&data))
	{}
	virtual ~cmd_fill_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdFillBuffer(command_buffer, buffer, offset, size, data);
	}
};

}
}
