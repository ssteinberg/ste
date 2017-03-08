//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_copy_image_to_buffer : public vk_command {
private:
	VkImage src_image;
	VkBuffer dst_buffer;
	VkImageLayout image_layout;

	std::vector<VkBufferImageCopy> ranges;

public:
	template <typename T, int d>
	vk_cmd_copy_image_to_buffer(const vk_image<d> &src_image,
								const vk_buffer<T> &dst_buffer,
								const VkImageLayout &image_layout,
								const std::vector<VkBufferImageCopy> &ranges = {})
		: src_image(src_image), dst_buffer(dst_buffer), image_layout(image_layout), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ src_image.get_size().x, src_image.get_size().y, src_image.get_size().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~vk_cmd_copy_image_to_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdCopyImageToBuffer(command_buffer, src_image, image_layout,
							   dst_buffer,
							   ranges.size(), ranges.data());
	}
};

}
}
