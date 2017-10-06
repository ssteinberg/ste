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

class cmd_copy_buffer_to_image : public command {
private:
	VkBuffer src_buffer;
	VkImage dst_image;
	image_layout layout;

	lib::vector<VkBufferImageCopy> ranges;

public:
	cmd_copy_buffer_to_image(cmd_copy_buffer_to_image &&) = default;
	cmd_copy_buffer_to_image(const cmd_copy_buffer_to_image &) = default;
	cmd_copy_buffer_to_image &operator=(cmd_copy_buffer_to_image &&) = default;
	cmd_copy_buffer_to_image &operator=(const cmd_copy_buffer_to_image &) = default;

	cmd_copy_buffer_to_image(const device_buffer_base &src_buffer,
							 const device_image_base &dst_image,
							 const image_layout &layout,
							 const lib::vector<buffer_image_copy_region_t> &ranges = {})
		: src_buffer(src_buffer.get_buffer_handle()), dst_image(dst_image.get_image_handle()), layout(layout) {
		for (auto &c : ranges)
			this->ranges.push_back(c.vk_descriptor(src_buffer.get_element_size_bytes()));

		if (this->ranges.size() == 0) {
			buffer_image_copy_region_t r;
			r.extent = dst_image.get_extent();
			r.image_format = dst_image.get_format();

			this->ranges.push_back(r.vk_descriptor(src_buffer.get_element_size_bytes()));
		}
	}

	virtual ~cmd_copy_buffer_to_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdCopyBufferToImage(command_buffer,
							   src_buffer,
							   dst_image,
							   static_cast<VkImageLayout>(layout),
							   static_cast<std::uint32_t>(ranges.size()),
							   ranges.data());
	}
};

}
}
