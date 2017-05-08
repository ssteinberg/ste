//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_clear_depth_stencil_image : public command {
private:
	std::reference_wrapper<const device_image_base> image;
	image_layout layout;
	VkClearDepthStencilValue clear_value{};

	std::vector<VkImageSubresourceRange> ranges;

public:
	cmd_clear_depth_stencil_image(const device_image_base &image,
								  const image_layout &layout,
								  float clear_depth = .0f,
								  const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_DEPTH_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		clear_value.depth = clear_depth;
	}
	virtual ~cmd_clear_depth_stencil_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdClearDepthStencilImage(command_buffer, image.get().get_image_handle(), static_cast<VkImageLayout>(layout),
									&clear_value,
									ranges.size(), ranges.data());
	}
};

}
}
