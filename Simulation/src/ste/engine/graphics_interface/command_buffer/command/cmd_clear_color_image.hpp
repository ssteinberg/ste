//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>

#include <image_layout.hpp>
#include <image_subresource_range.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_clear_color_image : public command {
private:
	VkImage image;
	image_layout layout;
	VkClearColorValue clear_value{};

	lib::vector<VkImageSubresourceRange> ranges;

private:
	void create_ranges(const lib::vector<image_subresource_range> &ranges) {
		for (auto &r : ranges)
			this->ranges.push_back(r.vk_descriptor());

		if (this->ranges.size() == 0) {
			image_subresource_range r;
			r.image_format = gl::format::r8g8b8a8_unorm;

			this->ranges.push_back(r.vk_descriptor());
		}
	}

public:
	cmd_clear_color_image(cmd_clear_color_image &&) = default;
	cmd_clear_color_image(const cmd_clear_color_image&) = default;
	cmd_clear_color_image &operator=(cmd_clear_color_image &&) = default;
	cmd_clear_color_image &operator=(const cmd_clear_color_image&) = default;

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::i32vec4 &clear_color,
						  const lib::vector<image_subresource_range> &ranges = {})
		: image(image.get_image_handle()), layout(layout) {
		create_ranges(ranges);
		memcpy(clear_value.int32, &clear_color.x, sizeof(clear_value.int32));
	}

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::u32vec4 &clear_color,
						  const lib::vector<image_subresource_range> &ranges = {})
		: image(image.get_image_handle()), layout(layout) {
		create_ranges(ranges);
		memcpy(clear_value.uint32, &clear_color.x, sizeof(clear_value.uint32));
	}

	cmd_clear_color_image(const device_image_base &image,
						  const image_layout &layout,
						  const glm::f32vec4 &clear_color,
						  const lib::vector<image_subresource_range> &ranges = {})
		: image(image.get_image_handle()), layout(layout) {
		create_ranges(ranges);
		memcpy(clear_value.float32, &clear_color.x, sizeof(clear_value.float32));
	}

	virtual ~cmd_clear_color_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
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
