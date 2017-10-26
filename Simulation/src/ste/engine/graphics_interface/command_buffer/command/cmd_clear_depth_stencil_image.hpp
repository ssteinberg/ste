//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class cmd_clear_depth_stencil_image : public command {
private:
	VkImage image;
	image_layout layout;
	VkClearDepthStencilValue clear_value{};

	lib::vector<VkImageSubresourceRange> ranges;

private:
	void create_ranges(const lib::vector<image_subresource_range> &ranges) {
		for (auto &r : ranges)
			this->ranges.push_back(r.vk_descriptor());

		if (this->ranges.size() == 0) {
			image_subresource_range r;
			r.image_format = gl::format::d32_sfloat;

			this->ranges.push_back(r.vk_descriptor());
		}
	}

public:
	cmd_clear_depth_stencil_image(cmd_clear_depth_stencil_image &&) = default;
	cmd_clear_depth_stencil_image(const cmd_clear_depth_stencil_image&) = default;
	cmd_clear_depth_stencil_image &operator=(cmd_clear_depth_stencil_image &&) = default;
	cmd_clear_depth_stencil_image &operator=(const cmd_clear_depth_stencil_image&) = default;

	cmd_clear_depth_stencil_image(const device_image_base &image,
								  const image_layout &layout,
								  float clear_depth = .0f,
								  const lib::vector<image_subresource_range> &ranges = {})
		: image(image.get_image_handle()), layout(layout) {
		create_ranges(ranges);
		clear_value.depth = clear_depth;
	}

	virtual ~cmd_clear_depth_stencil_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdClearDepthStencilImage(command_buffer,
									image,
									static_cast<VkImageLayout>(layout),
									&clear_value,
									static_cast<std::uint32_t>(ranges.size()),
									ranges.data());
	}
};

}
}
