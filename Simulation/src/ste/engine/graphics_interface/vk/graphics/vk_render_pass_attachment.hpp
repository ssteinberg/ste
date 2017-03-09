//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class vk_render_pass_attachment {
private:
	VkFormat format;
	VkImageLayout initial_layout;
	VkImageLayout final_layout;

	VkAttachmentLoadOp load_op;
	VkAttachmentStoreOp store_op;

public:
	vk_render_pass_attachment(VkFormat format,
							  VkImageLayout initial_layout,
							  VkImageLayout final_layout,
							  VkAttachmentLoadOp load_op,
							  VkAttachmentStoreOp store_op)
		: format(format), initial_layout(initial_layout), final_layout(final_layout),
		load_op(load_op), store_op(store_op)
	{
		assert(final_layout != VK_IMAGE_LAYOUT_UNDEFINED);
	}

	static auto load_and_store(VkFormat format,
							   VkImageLayout initial_layout,
							   VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 initial_layout,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_LOAD,
										 VK_ATTACHMENT_STORE_OP_STORE);
	}
	static auto load_and_discard(VkFormat format,
								 VkImageLayout initial_layout,
								 VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 initial_layout,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_LOAD,
										 VK_ATTACHMENT_STORE_OP_DONT_CARE);
	}
	static auto clear_and_store(VkFormat format,
								VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 VK_IMAGE_LAYOUT_UNDEFINED,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_CLEAR,
										 VK_ATTACHMENT_STORE_OP_STORE);
	}
	static auto clear_and_discard(VkFormat format,
								  VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 VK_IMAGE_LAYOUT_UNDEFINED,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_CLEAR,
										 VK_ATTACHMENT_STORE_OP_DONT_CARE);
	}
	static auto ignore_and_store(VkFormat format,
								 VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 VK_IMAGE_LAYOUT_UNDEFINED,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_DONT_CARE,
										 VK_ATTACHMENT_STORE_OP_STORE);
	}
	static auto ignore_and_discard(VkFormat format,
								   VkImageLayout final_layout) {
		return vk_render_pass_attachment(format,
										 VK_IMAGE_LAYOUT_UNDEFINED,
										 final_layout,
										 VK_ATTACHMENT_LOAD_OP_DONT_CARE,
										 VK_ATTACHMENT_STORE_OP_DONT_CARE);
	}

	operator VkAttachmentDescription() const {
		VkAttachmentDescription attachment = {};
		attachment.flags = 0;
		attachment.format = format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = load_op;
		attachment.storeOp = store_op;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = initial_layout;
		attachment.finalLayout = final_layout;

		return attachment;
	}
};

}
}
