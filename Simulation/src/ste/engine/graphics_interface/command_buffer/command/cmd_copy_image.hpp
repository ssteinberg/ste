//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_image.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_copy_image : public command {
private:
	std::reference_wrapper<const vk::vk_image> src_image;
	VkImageLayout src_image_layout;
	std::reference_wrapper<const vk::vk_image> dst_image;
	VkImageLayout dst_image_layout;

	std::vector<VkImageCopy> ranges;

public:
	cmd_copy_image(const vk::vk_image &src_image,
				   const VkImageLayout &src_image_layout,
				   const vk::vk_image &dst_image,
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
		vkCmdCopyImage(command_buffer, src_image.get(), src_image_layout,
					   dst_image.get(), dst_image_layout,
					   ranges.size(), ranges.data());
	}
};

}
}
