//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>
#include <vk_image.hpp>

#include <vector>
#include <functional>

namespace ste {
namespace gl {

class cmd_copy_buffer : public command {
private:
	std::reference_wrapper<const vk::vk_buffer> src_buffer;
	std::reference_wrapper<const vk::vk_buffer> dst_buffer;

	std::vector<VkBufferCopy> ranges;

public:
	cmd_copy_buffer(const vk::vk_buffer &src_buffer,
					const vk::vk_buffer &dst_buffer,
					const std::vector<VkBufferCopy> &ranges = {})
		: src_buffer(src_buffer), dst_buffer(dst_buffer), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferCopy c = {
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
		vkCmdCopyBuffer(command_buffer, src_buffer.get(),
						dst_buffer.get(),
						ranges.size(), ranges.data());
	}
};

}
}
