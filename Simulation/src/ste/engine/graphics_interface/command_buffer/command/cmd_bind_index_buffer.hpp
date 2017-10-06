//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>

namespace ste {
namespace gl {

class cmd_bind_index_buffer : public command {
private:
	VkBuffer buffer;
	std::uint64_t offset;
	VkIndexType index_type;

public:
	cmd_bind_index_buffer(cmd_bind_index_buffer &&) = default;
	cmd_bind_index_buffer(const cmd_bind_index_buffer&) = default;
	cmd_bind_index_buffer &operator=(cmd_bind_index_buffer &&) = default;
	cmd_bind_index_buffer &operator=(const cmd_bind_index_buffer&) = default;

	cmd_bind_index_buffer(const device_buffer_base &buffer,
						  std::uint64_t offset = 0)
		: buffer(buffer.get_buffer_handle()),
		  offset(offset), index_type(VK_INDEX_TYPE_UINT32) {
		if (buffer.get_element_size_bytes() == 2_B) {
			index_type = VK_INDEX_TYPE_UINT16;
			this->offset *= 2;
		}
		else if (buffer.get_element_size_bytes() == 4_B) {
			index_type = VK_INDEX_TYPE_UINT32;
			this->offset *= 4;
		}
		else
			assert(false && "Expected a uint16 or uint32 buffer");
	}

	virtual ~cmd_bind_index_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdBindIndexBuffer(command_buffer, buffer, offset, index_type);
	}
};

}
}
