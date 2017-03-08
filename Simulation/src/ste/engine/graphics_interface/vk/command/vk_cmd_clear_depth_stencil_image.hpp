//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_clear_depth_stencil_image : public vk_command {
private:
	VkImage image;
	VkImageLayout layout;
	VkClearDepthStencilValue clear_value{};

	std::vector<VkImageSubresourceRange> ranges;

public:
	template <int d>
	vk_cmd_clear_depth_stencil_image(const vk_image<d> &image,
									 const VkImageLayout &layout,
									 float clear_depth = .0f,
									 const std::vector<VkImageSubresourceRange> &ranges = {})
		: image(image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0)
			this->ranges.push_back({ VK_IMAGE_ASPECT_DEPTH_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS });
		clear_value.depth = clear_depth;
	}
	virtual ~vk_cmd_clear_depth_stencil_image() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdClearDepthStencilImage(command_buffer, image, layout,
									&clear_value,
									ranges.size(), ranges.data());
	}
};

}
}
