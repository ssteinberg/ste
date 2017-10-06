//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>

#include <buffer_image_copy_region.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_copy_image_to_buffer : public command {
private:
	VkImage src_image;
	VkBuffer dst_buffer;
	image_layout layout;

	lib::vector<VkBufferImageCopy> ranges;

public:
	cmd_copy_image_to_buffer(cmd_copy_image_to_buffer &&) = default;
	cmd_copy_image_to_buffer(const cmd_copy_image_to_buffer&) = default;
	cmd_copy_image_to_buffer &operator=(cmd_copy_image_to_buffer &&) = default;
	cmd_copy_image_to_buffer &operator=(const cmd_copy_image_to_buffer&) = default;

	cmd_copy_image_to_buffer(const device_image_base &src_image,
							 const image_layout &layout,
							 const device_buffer_base &dst_buffer,
							 const lib::vector<buffer_image_copy_region_t> &ranges = {})
		: src_image(src_image.get_image_handle()), dst_buffer(dst_buffer.get_buffer_handle()), layout(layout) {
		for (auto &c : ranges)
			this->ranges.push_back(c.vk_descriptor(dst_buffer.get_element_size_bytes()));

		if (this->ranges.size() == 0) {
			buffer_image_copy_region_t r;
			r.extent = src_image.get_extent();
			r.image_format = src_image.get_format();

			this->ranges.push_back(r.vk_descriptor(dst_buffer.get_element_size_bytes()));
		}
	}

	virtual ~cmd_copy_image_to_buffer() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdCopyImageToBuffer(command_buffer,
							   src_image,
							   static_cast<VkImageLayout>(layout),
							   dst_buffer,
							   static_cast<std::uint32_t>(ranges.size()),
							   ranges.data());
	}
};

}
}
