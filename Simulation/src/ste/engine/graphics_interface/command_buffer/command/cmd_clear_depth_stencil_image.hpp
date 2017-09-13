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

class cmd_clear_depth_stencil_image : public command {
private:
	VkImage image;
	image_layout layout;
	VkClearDepthStencilValue clear_value{};

	lib::vector<VkImageSubresourceRange> ranges;

public:
	cmd_clear_depth_stencil_image(cmd_clear_depth_stencil_image &&) = default;
	cmd_clear_depth_stencil_image(const cmd_clear_depth_stencil_image&) = default;
	cmd_clear_depth_stencil_image &operator=(cmd_clear_depth_stencil_image &&) = default;
	cmd_clear_depth_stencil_image &operator=(const cmd_clear_depth_stencil_image&) = default;

	cmd_clear_depth_stencil_image(const device_image_base &image,
								  const image_layout &layout,
								  float clear_depth = .0f,
								  const lib::vector<VkImageSubresourceRange> &ranges = {})
		: image(image.get_image_handle()), layout(layout), ranges(ranges) {
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_DEPTH_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		clear_value.depth = clear_depth;
	}

	virtual ~cmd_clear_depth_stencil_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdClearDepthStencilImage(command_buffer,
									image,
									static_cast<VkImageLayout>(layout),
									&clear_value,
									static_cast<std::uint32_t>(ranges.size()),
									ranges.data());
	}
};

}
}
