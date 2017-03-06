//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional.hpp>

namespace StE {
namespace GL {

class vk_render_pass_subpass_descriptor {
	friend class vk_render_pass;

private:
	std::vector<VkAttachmentReference> color;
	std::vector<VkAttachmentReference> input;
	optional<VkAttachmentReference> depth;
	std::vector<std::uint32_t> preserve;

public:
	vk_render_pass_subpass_descriptor(const std::vector<VkAttachmentReference> &color) 
		: color(color) {}

	vk_render_pass_subpass_descriptor(const std::vector<VkAttachmentReference> &color,
									  const std::vector<VkAttachmentReference> &input)
		: color(color), input(input) {}

	vk_render_pass_subpass_descriptor(const std::vector<VkAttachmentReference> &color,
									  const std::vector<VkAttachmentReference> &input,
									  VkAttachmentReference depth,
									  const std::vector<std::uint32_t> &preserve = {})
		: color(color), input(input), depth(depth), preserve(preserve) {}

	vk_render_pass_subpass_descriptor(const std::vector<VkAttachmentReference> &color,
									  VkAttachmentReference depth)
		: color(color), depth(depth) {}
};

}
}
