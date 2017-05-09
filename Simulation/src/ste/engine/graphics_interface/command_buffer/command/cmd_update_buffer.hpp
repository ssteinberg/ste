//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <buffer_view.hpp>

#include <blob.hpp>

namespace ste {
namespace gl {

class cmd_update_buffer : public command {
private:
	buffer_view buffer;
	blob data;

public:
	template <typename Blob>
	cmd_update_buffer(const buffer_view &buffer,
					  Blob&& data)
		: buffer(buffer),
		data(std::forward<Blob>(data))
	{
		assert(this->data.size() <= buffer.range_bytes());
	}
	virtual ~cmd_update_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		VkBuffer handle = buffer.get().get_buffer_handle();
		vkCmdUpdateBuffer(command_buffer, 
						  handle,
						  buffer.offset_bytes(), 
						  data.size(), 
						  data.data());
	}
};

}
}
