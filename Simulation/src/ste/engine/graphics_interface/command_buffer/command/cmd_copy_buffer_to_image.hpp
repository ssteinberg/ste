//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>
#include <device_image_base.hpp>

#include <image_layout.hpp>
#include <format_rtti.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_copy_buffer_to_image : public command {
private:
	std::reference_wrapper<const device_buffer_base> src_buffer;
	std::reference_wrapper<const device_image_base> dst_image;
	image_layout layout;

	std::vector<VkBufferImageCopy> ranges;

public:
	cmd_copy_buffer_to_image(const device_buffer_base &src_buffer,
							 const device_image_base &dst_image,
							 const image_layout &layout,
							 const std::vector<VkBufferImageCopy> &ranges = {})
		: src_buffer(src_buffer), dst_image(dst_image), layout(layout), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ format_aspect(dst_image.get_format()), 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ dst_image.get_size().x, dst_image.get_size().y, dst_image.get_size().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~cmd_copy_buffer_to_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyBufferToImage(command_buffer, src_buffer.get().get_buffer_handle(),
							   dst_image.get().get_image_handle(), static_cast<VkImageLayout>(layout),
							   ranges.size(), ranges.data());
	}
};

}
}
