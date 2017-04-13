//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <buffer_view.hpp>

#include <string>

namespace StE {
namespace GL {

class cmd_update_buffer : public command {
private:
	buffer_view buffer;
	std::string data;

public:
	cmd_update_buffer(const buffer_view &buffer,
					  std::uint64_t data_size,
					  const void *data)
		: buffer(buffer),
		data(reinterpret_cast<const char*>(data), static_cast<std::size_t>(data_size))
	{}
	virtual ~cmd_update_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdUpdateBuffer(command_buffer, 
						  buffer->get_buffer_handle(), 
						  buffer.offset_bytes(), 
						  std::min<std::uint64_t>(buffer.range_bytes(), data.size()), 
						  data.data());
	}
};

}
}
