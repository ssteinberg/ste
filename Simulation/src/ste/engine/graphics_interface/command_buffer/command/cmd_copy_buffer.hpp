//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>

#include <lib/vector.hpp>
#include <functional>

namespace ste {
namespace gl {

class cmd_copy_buffer : public command {
private:
	VkBuffer src_buffer;
	VkBuffer dst_buffer;

	lib::vector<VkBufferCopy> ranges;

public:
	cmd_copy_buffer(cmd_copy_buffer&&) = default;
	cmd_copy_buffer(const cmd_copy_buffer&) = default;
	cmd_copy_buffer &operator=(cmd_copy_buffer&&) = default;
	cmd_copy_buffer &operator=(const cmd_copy_buffer&) = default;

	cmd_copy_buffer(const device_buffer_base &src_buffer,
					const device_buffer_base &dst_buffer,
					const lib::vector<VkBufferCopy> &ranges = {})
		: src_buffer(src_buffer.get_buffer_handle()), dst_buffer(dst_buffer.get_buffer_handle()), ranges(ranges) {
		if (this->ranges.size() == 0) {
			const VkBufferCopy c = {
				0, 0,
				std::min(src_buffer.get_elements_count() * src_buffer.get_element_size_bytes(),
						 dst_buffer.get_elements_count() * dst_buffer.get_element_size_bytes())
			};
			this->ranges.push_back(c);
		}
	}

	virtual ~cmd_copy_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyBuffer(command_buffer,
						src_buffer,
						dst_buffer,
						static_cast<std::uint32_t>(ranges.size()),
						ranges.data());
	}
};

}
}
