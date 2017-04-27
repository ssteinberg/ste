//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_clear_color_image : public command {
private:
	std::reference_wrapper<const vk::vk_image> image;
	VkImageLayout layout;
	VkClearColorValue clear_value{};

	std::vector<VkImageSubresourceRange> ranges;

public:
	cmd_clear_color_image(const vk::vk_image &image,
						  const VkImageLayout &layout,
						  const glm::i32vec4 &clear_color,
						  const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.int32, &clear_color.x, sizeof(clear_value.int32));
	}
	cmd_clear_color_image(const vk::vk_image &image,
						  const VkImageLayout &layout,
						  const glm::u32vec4 &clear_color,
						  const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.uint32, &clear_color.x, sizeof(clear_value.uint32));
	}
	cmd_clear_color_image(const vk::vk_image &image,
						  const VkImageLayout &layout,
						  const glm::f32vec4 &clear_color,
						  const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.float32, &clear_color.x, sizeof(clear_value.float32));
	}
	virtual ~cmd_clear_color_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdClearColorImage(command_buffer, image.get(), layout,
							 &clear_value,
							 ranges.size(), ranges.data());
	}
};

}
}
