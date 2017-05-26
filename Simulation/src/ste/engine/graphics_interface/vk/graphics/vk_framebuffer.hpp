//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_render_pass.hpp>

#include <vk_host_allocator.hpp>
#include <optional.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_framebuffer : public allow_type_decay<vk_framebuffer<host_allocator>, VkFramebuffer> {
private:
	optional<VkFramebuffer> framebuffer;
	alias<const vk_logical_device<host_allocator>> device;
	glm::u32vec2 extent;

public:
	vk_framebuffer(const vk_logical_device<host_allocator> &device,
				   const vk_render_pass<host_allocator> &render_pass,
				   const lib::vector<VkImageView> &attachments,
				   const glm::u32vec2 &extent) : device(device), extent(extent) {
		VkFramebuffer fb;

		VkFramebufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.renderPass = render_pass;
		create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.width = extent.x;
		create_info.height = extent.y;
		create_info.layers = 1;

		vk_result res = vkCreateFramebuffer(device, &create_info, &host_allocator::allocation_callbacks(), &fb);
		if (!res) {
			throw vk_exception(res);
		}

		this->framebuffer = fb;
	}
	~vk_framebuffer() noexcept { destroy_framebuffer(); }

	vk_framebuffer(vk_framebuffer &&) = default;
	vk_framebuffer &operator=(vk_framebuffer &&o) noexcept {
		destroy_framebuffer();

		framebuffer = std::move(o.framebuffer);
		device = std::move(o.device);
		extent = o.extent;

		return *this;
	}
	vk_framebuffer(const vk_framebuffer &) = delete;
	vk_framebuffer &operator=(const vk_framebuffer &) = delete;

	void destroy_framebuffer() {
		if (framebuffer) {
			vkDestroyFramebuffer(device.get(), *this, &host_allocator::allocation_callbacks());
			framebuffer = none;
		}
	}

	auto& get() const { return framebuffer.get(); }
};

}

}
}
