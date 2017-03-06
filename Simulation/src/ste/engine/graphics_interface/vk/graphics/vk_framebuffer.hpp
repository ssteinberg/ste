//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_render_pass.hpp>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_framebuffer {
private:
	optional<VkFramebuffer> framebuffer;
	const vk_logical_device &device;

public:
	vk_framebuffer(const vk_logical_device &device,
				   const vk_render_pass &render_pass) : device(device) {
		VkFramebuffer fb;

		VkFramebufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.renderPass = render_pass;

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
			vkDestroyFramebuffer(device, *this, nullptr);
			framebuffer = none;
		}
	}

	auto& get_framebuffer() const { return framebuffer.get(); }

	operator VkFramebuffer() const { return get_framebuffer(); }
};

}
}
