//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_clear_color_image : public vk_command {
private:
	VkImage image;
	VkImageLayout layout;
	VkClearColorValue clear_value{};

	std::vector<VkImageSubresourceRange> ranges;

public:
	template <int d>
	vk_cmd_clear_color_image(const vk_image<d> &image,
							 const VkImageLayout &layout,
							 const glm::i32vec4 &clear_color,
							 const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.int32, &clear_color.x, sizeof(clear_value.int32));
	}
	template <int d>
	vk_cmd_clear_color_image(const vk_image<d> &image,
							 const VkImageLayout &layout,
							 const glm::u32vec4 &clear_color,
							 const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.uint32, &clear_color.x, sizeof(clear_value.uint32));
	}
	template <int d>
	vk_cmd_clear_color_image(const vk_image<d> &image,
							 const VkImageLayout &layout,
							 const glm::f32vec4 &clear_color,
							 const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.float32, &clear_color.x, sizeof(clear_value.float32));
	}
	virtual ~vk_cmd_clear_color_image() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdClearColorImage(command_buffer, image, layout, 
							 &clear_value, 
							 ranges.size(), ranges.data());
	}
};

}
}
