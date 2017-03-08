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

class vk_cmd_copy_buffer_to_image : public vk_command {
private:
	VkBuffer src_buffer;
	VkImage dst_image;
	VkImageLayout image_layout;

	std::vector<VkBufferImageCopy> ranges;

public:
	template <typename T, int d>
	vk_cmd_copy_buffer_to_image(const vk_buffer<T> &src_buffer,
								const vk_image<d> &dst_image,
								const VkImageLayout &image_layout,
								const std::vector<VkBufferImageCopy> &ranges = {})
		: src_buffer(src_buffer), dst_image(dst_image), image_layout(image_layout), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ dst_image.get_size().x, dst_image.get_size().y, dst_image.get_size().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~vk_cmd_copy_buffer_to_image() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdCopyBufferToImage(command_buffer, src_buffer, 
							   dst_image, image_layout,
							   ranges.size(), ranges.data());
	}
};

}
}
