//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_render_pass.hpp>

#include <optional.hpp>

#include <vector>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class vk_framebuffer : public allow_class_decay<vk_framebuffer, VkFramebuffer> {
private:
	optional<VkFramebuffer> framebuffer;
	std::reference_wrapper<const vk_logical_device> device;
	glm::u32vec2 extent;

public:
	vk_framebuffer(const vk_logical_device &device,
				   const vk_render_pass &render_pass,
				   const std::vector<VkImageView> &attachments,
				   const glm::u32vec2 &extent) : device(device), extent(extent) {
		VkFramebuffer fb;

		VkFramebufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.renderPass = render_pass;
		create_info.attachmentCount = attachments.size();
		create_info.pAttachments = attachments.data();
		create_info.width = extent.x;
		create_info.height = extent.y;
		create_info.layers = 1;

		vk_result res = vkCreateFramebuffer(device, &create_info, nullptr, &fb);
		if (!res) {
			throw vk_exception(res);
		}

		this->framebuffer = fb;
	}
	~vk_framebuffer() noexcept { destroy_framebuffer(); }

	vk_framebuffer(vk_framebuffer &&) = default;
	vk_framebuffer &operator=(vk_framebuffer &&) = default;
	vk_framebuffer(const vk_framebuffer &) = delete;
	vk_framebuffer &operator=(const vk_framebuffer &) = delete;

	void destroy_framebuffer() {
		if (framebuffer) {
			vkDestroyFramebuffer(device.get(), *this, nullptr);
			framebuffer = none;
		}
	}

	auto& get() const { return framebuffer.get(); }
};

}
}
