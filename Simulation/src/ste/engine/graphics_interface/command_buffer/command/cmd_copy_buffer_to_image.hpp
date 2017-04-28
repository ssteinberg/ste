//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>
#include <vk_image.hpp>

#include <image_layout.hpp>
#include <format_rtti.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_copy_buffer_to_image : public command {
private:
	std::reference_wrapper<const vk::vk_buffer> src_buffer;
	std::reference_wrapper<const vk::vk_image> dst_image;
	image_layout layout;

	std::vector<VkBufferImageCopy> ranges;

public:
	cmd_copy_buffer_to_image(const vk::vk_buffer &src_buffer,
							 const vk::vk_image &dst_image,
							 const image_layout &layout,
							 const std::vector<VkBufferImageCopy> &ranges = {})
		: src_buffer(src_buffer), dst_image(dst_image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ vk_format_aspect(dst_image.get_format()), 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ dst_image.get_size().x, dst_image.get_size().y, dst_image.get_size().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~cmd_copy_buffer_to_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyBufferToImage(command_buffer, src_buffer.get(),
							   dst_image.get(), static_cast<VkImageLayout>(layout),
							   ranges.size(), ranges.data());
	}
};

}
}
