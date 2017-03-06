//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_render_pass {
private:
	optional<VkRenderPass> render_pass;
	const vk_logical_device &device;

public:
	vk_render_pass(const vk_logical_device &device) : device(device) {
		VkRenderPass fb;

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
//		create_info.

		vk_result res = vkCreateRenderPass(device, &create_info, nullptr, &fb);
		if (!res) {
			throw vk_exception(res);
		}

		this->render_pass = fb;
	}
	~vk_render_pass() noexcept { destroy_render_pass(); }

	vk_render_pass(vk_render_pass &&) = default;
	vk_render_pass &operator=(vk_render_pass &&) = default;
	vk_render_pass(const vk_render_pass &) = delete;
	vk_render_pass &operator=(const vk_render_pass &) = delete;

	void destroy_render_pass() {
		if (render_pass) {
			vkDestroyRenderPass(device, *this, nullptr);
			render_pass = none;
		}
	}

	auto& get_render_pass() const { return render_pass.get(); }

	operator VkRenderPass() const { return get_render_pass(); }
};

}
}
