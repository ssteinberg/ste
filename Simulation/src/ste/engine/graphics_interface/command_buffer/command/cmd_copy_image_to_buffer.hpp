//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>
#include <device_image_base.hpp>

#include <image_layout.hpp>
#include <format_rtti.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_copy_image_to_buffer : public command {
private:
	std::reference_wrapper<const device_image_base> src_image;
	std::reference_wrapper<const device_buffer_base> dst_buffer;
	image_layout layout;

	lib::vector<VkBufferImageCopy> ranges;

public:
	cmd_copy_image_to_buffer(const device_image_base &src_image,
							 const image_layout &layout,
							 const device_buffer_base &dst_buffer,
							 const lib::vector<VkBufferImageCopy> &ranges = {})
		: src_image(src_image), dst_buffer(dst_buffer), layout(layout), ranges(ranges)
	{
		for (auto &c : this->ranges) {
			c.bufferImageHeight *= dst_buffer.get_element_size_bytes();
			c.bufferOffset *= dst_buffer.get_element_size_bytes();
			c.bufferRowLength *= dst_buffer.get_element_size_bytes();
		}

		if (this->ranges.size() == 0) {
			VkBufferImageCopy c = {
				0, 0, 0,
				{ static_cast<VkImageAspectFlags>(format_aspect(src_image.get_format())), 0, 0, VK_REMAINING_ARRAY_LAYERS },
				{ 0, 0, 0 },
				{ src_image.get_extent().x, src_image.get_extent().y, src_image.get_extent().z }
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~cmd_copy_image_to_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdCopyImageToBuffer(command_buffer, src_image.get().get_image_handle(), static_cast<VkImageLayout>(layout),
							   dst_buffer.get().get_buffer_handle(),
							   static_cast<std::uint32_t>(ranges.size()),
							   ranges.data());
	}
};

}
}
