//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class cmd_copy_image : public command {
private:
	VkImage src_image;
	VkImageLayout src_image_layout;
	VkImage dst_image;
	VkImageLayout dst_image_layout;

	std::vector<VkImageCopy> ranges;

public:
	template <int d1, int d2>
	cmd_copy_image(const vk_image<d1> &src_image,
				   const VkImageLayout &src_image_layout,
				   const vk_image<d2> &dst_image,
				   const VkImageLayout &dst_image_layout,
				   const std::vector<VkImageCopy> &ranges = {})
		: src_image(src_image), src_image_layout(src_image_layout),
		dst_image(dst_image), dst_image_layout(dst_image_layout),
		ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			assert(false);
		}
	}
	virtual ~cmd_copy_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyImage(command_buffer, src_image, src_image_layout,
					   dst_image, dst_image_layout,
					   ranges.size(), ranges.data());
	}
};

}
}
