//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <buffer_view.hpp>

#include <lib/blob.hpp>

namespace ste {
namespace gl {

class cmd_update_buffer : public command {
public:
	static constexpr auto maximal_update_bytes = 64_kB;

private:
	VkBuffer buffer;
	byte_t offset;
	lib::blob data;

public:
	cmd_update_buffer(cmd_update_buffer &&) = default;
	cmd_update_buffer(const cmd_update_buffer&) = default;
	cmd_update_buffer &operator=(cmd_update_buffer &&) = default;
	cmd_update_buffer &operator=(const cmd_update_buffer&) = default;

	template <typename Blob>
	cmd_update_buffer(const buffer_view &buffer,
					  Blob &&data)
		: buffer(buffer->get_buffer_handle()),
		  offset(buffer.offset_bytes()),
		  data(std::forward<Blob>(data)) {
		assert(byte_t(this->data.size()) <= buffer.range_bytes());
	}

	virtual ~cmd_update_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {

		vkCmdUpdateBuffer(command_buffer,
						  buffer,
						  static_cast<std::size_t>(offset),
						  data.size(),
						  data.data());
	}
};

}
}
