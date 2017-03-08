//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_blit_image : public vk_command {
private:
	VkImage src_image;
	VkImageLayout src_image_layout;
	VkImage dst_image;
	VkImageLayout dst_image_layout;

	std::vector<VkImageBlit> ranges;

	VkFilter filter;

public:
	template <int d1, int d2>
	vk_cmd_blit_image(const vk_image<d1> &src_image,
					  const VkImageLayout &src_image_layout,
					  const vk_image<d1> &dst_image,
					  const VkImageLayout &dst_image_layout,
					  VkFilter filter,
					  const std::vector<VkImageBlit> &ranges = {})
		: src_image(src_image), src_image_layout(src_image_layout),
		dst_image(dst_image), dst_image_layout(dst_image_layout),
		ranges(ranges), filter(filter)
	{
		if (this->ranges.size() == 0) {
			VkImageBlit c = {
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~vk_cmd_blit_image() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBlitImage(command_buffer, src_image, src_image_layout,
					   dst_image, dst_image_layout,
					   ranges.size(), ranges.data(),
					   filter);
	}
};

}
}
