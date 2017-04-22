//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer_base.hpp>
#include <vk_image.hpp>

#include <vk_format_rtti.hpp>

#include <vector>

namespace StE {
namespace GL {

class cmd_copy_image_to_buffer : public command {
private:
	VkImage src_image;
	VkBuffer dst_buffer;
	VkImageLayout image_layout;

	std::vector<VkBufferImageCopy> ranges;

public:
	template <int d>
	cmd_copy_image_to_buffer(const vk_image<d> &src_image,
							 const vk_buffer_base &dst_buffer,
							 const VkImageLayout &image_layout,
							 const std::vector<VkBufferImageCopy> &ranges = {})
		: src_image(src_image), dst_buffer(dst_buffer), image_layout(image_layout), ranges(ranges)
	{
		for (auto &c : this->ranges) {
			c.bufferImageHeight *= dst_buffer.get_element_size_bytes();
			c.bufferOffset *= dst_buffer.get_element_size_bytes();
			c.bufferRowLength *= dst_buffer.get_element_size_bytes();
		}

		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ vk_format_aspect(src_image.get_format()), 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ src_image.get_size().x, src_image.get_size().y, src_image.get_size().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~cmd_copy_image_to_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyImageToBuffer(command_buffer, src_image, image_layout,
							   dst_buffer,
							   ranges.size(), ranges.data());
	}
};

}
}
