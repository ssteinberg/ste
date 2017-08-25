//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <lib/vector.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_render_pass_subpass_descriptor {
	template <typename>
	friend class vk_render_pass;

private:
	lib::vector<VkAttachmentReference> color;
	lib::vector<VkAttachmentReference> input;
	optional<VkAttachmentReference> depth;
	lib::vector<std::uint32_t> preserve;

public:
	vk_render_pass_subpass_descriptor(const lib::vector<VkAttachmentReference> &color)
		: color(color) {}

	vk_render_pass_subpass_descriptor(const lib::vector<VkAttachmentReference> &color,
									  const lib::vector<VkAttachmentReference> &input)
		: color(color), input(input) {}

	vk_render_pass_subpass_descriptor(const lib::vector<VkAttachmentReference> &color,
									  const lib::vector<VkAttachmentReference> &input,
									  VkAttachmentReference depth,
									  const lib::vector<std::uint32_t> &preserve = {})
		: color(color), input(input), depth(depth), preserve(preserve) {}

	vk_render_pass_subpass_descriptor(const lib::vector<VkAttachmentReference> &color,
									  VkAttachmentReference depth)
		: color(color), depth(depth) {}
};

}

}
}
