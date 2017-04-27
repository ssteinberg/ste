//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_clear_depth_stencil_image : public command {
private:
	std::reference_wrapper<const vk::vk_image> image;
	VkImageLayout layout;
	VkClearDepthStencilValue clear_value{};

	std::vector<VkImageSubresourceRange> ranges;

public:
	cmd_clear_depth_stencil_image(const vk::vk_image &image,
								  const VkImageLayout &layout,
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
		vkCmdClearDepthStencilImage(command_buffer, image.get(), layout,
									&clear_value,
									ranges.size(), ranges.data());
	}
};

}
}
