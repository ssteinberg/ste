//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <buffer_view.hpp>

namespace StE {
namespace GL {

class cmd_fill_buffer : public command {
private:
	buffer_view buffer;
	std::uint32_t data;

public:
	cmd_fill_buffer(const buffer_view &buffer,
					std::uint32_t data) : buffer(buffer), data(data)
	{}
	cmd_fill_buffer(const buffer_view &buffer,
					float data) : buffer(buffer), data(*reinterpret_cast<std::uint32_t*>(&data))
	{}
	virtual ~cmd_fill_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdFillBuffer(command_buffer, 
						buffer->get_buffer_handle(),
						buffer.offset_bytes(), 
						buffer.range_bytes(), 
						data);
	}
};

}
}
