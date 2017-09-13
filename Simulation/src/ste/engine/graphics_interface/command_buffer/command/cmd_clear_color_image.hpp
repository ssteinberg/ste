//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_clear_color_image : public command {
private:
	VkImage image;
	image_layout layout;
	VkClearColorValue clear_value{};

	lib::vector<VkImageSubresourceRange> ranges;

public:
	cmd_clear_color_image(cmd_clear_color_image &&) = default;
	cmd_clear_color_image(const cmd_clear_color_image&) = default;
	cmd_clear_color_image &operator=(cmd_clear_color_image &&) = default;
	cmd_clear_color_image &operator=(const cmd_clear_color_image&) = default;

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::i32vec4 &clear_color,
						  const lib::vector<VkImageSubresourceRange> &ranges = {})
		: image(image.get_image_handle()), layout(layout), ranges(ranges) {
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.int32, &clear_color.x, sizeof(clear_value.int32));
	}

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::u32vec4 &clear_color,
						  const lib::vector<VkImageSubresourceRange> &ranges = {})
		: image(image.get_image_handle()), layout(layout), ranges(ranges) {
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.uint32, &clear_color.x, sizeof(clear_value.uint32));
	}

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::f32vec4 &clear_color,
						  const lib::vector<VkImageSubresourceRange> &ranges = {})
		: image(image.get_image_handle()), layout(layout), ranges(ranges) {
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		memcpy(clear_value.float32, &clear_color.x, sizeof(clear_value.float32));
	}

	virtual ~cmd_clear_color_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdClearColorImage(command_buffer,
							 image,
							 static_cast<VkImageLayout>(layout),
							 &clear_value,
							 static_cast<std::uint32_t>(ranges.size()),
							 ranges.data());
	}
};

}
}
