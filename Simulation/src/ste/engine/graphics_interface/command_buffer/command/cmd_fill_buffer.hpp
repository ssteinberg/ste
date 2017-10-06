//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <buffer_view.hpp>

namespace ste {
namespace gl {

class cmd_fill_buffer : public command {
private:
	VkBuffer buffer;
	byte_t offset;
	byte_t range;
	std::uint32_t data;

public:
	cmd_fill_buffer(cmd_fill_buffer &&) = default;
	cmd_fill_buffer(const cmd_fill_buffer&) = default;
	cmd_fill_buffer &operator=(cmd_fill_buffer &&) = default;
	cmd_fill_buffer &operator=(const cmd_fill_buffer&) = default;

	cmd_fill_buffer(const buffer_view &buffer,
					std::uint32_t data)
		: buffer(buffer->get_buffer_handle()),
		  offset(buffer.offset_bytes()),
		  range(buffer.range_bytes()),
		  data(data) {}

	cmd_fill_buffer(const buffer_view &buffer,
					float data)
		: buffer(buffer->get_buffer_handle()),
		  offset(buffer.offset_bytes()),
		  range(buffer.range_bytes()),
		  data(*reinterpret_cast<std::uint32_t*>(&data)) {}

	virtual ~cmd_fill_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdFillBuffer(command_buffer,
						buffer,
						static_cast<std::size_t>(offset),
						static_cast<std::size_t>(range),
						data);
	}
};

}
}
