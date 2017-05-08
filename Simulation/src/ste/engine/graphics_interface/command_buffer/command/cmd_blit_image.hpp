//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_image_base.hpp>
#include <image_layout.hpp>
#include <sampler_filter.hpp>

#include <vector>

namespace ste {
namespace gl {

class cmd_blit_image : public command {
private:
	std::reference_wrapper<const device_image_base> src_image;
	image_layout src_image_layout;
	std::reference_wrapper<const device_image_base> dst_image;
	image_layout dst_image_layout;

	std::vector<VkImageBlit> ranges;

	sampler_filter filter;

public:
	cmd_blit_image(const device_image_base &src_image,
				   const image_layout &src_image_layout,
				   const device_image_base &dst_image,
				   const image_layout &dst_image_layout,
				   sampler_filter filter = sampler_filter::linear,
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
	virtual ~cmd_blit_image() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdBlitImage(command_buffer, src_image.get().get_image_handle(), static_cast<VkImageLayout>(src_image_layout),
					   dst_image.get().get_image_handle(), static_cast<VkImageLayout>(dst_image_layout),
					   ranges.size(), ranges.data(),
					   static_cast<VkFilter>(filter));
	}
};

}
}
