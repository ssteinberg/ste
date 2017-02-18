//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>
#include <optional.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_surface.hpp>

namespace StE {
namespace GL {

class vk_swapchain {
private:
	VkSwapchainCreateInfoKHR swapchain_create_info;
	optional<VkSwapchainKHR> swapchain;
	const vk_logical_device &device;

public:
	vk_swapchain(const vk_logical_device &device,
				 const vk_surface &surface,
				 std::uint32_t min_image_count,
				 const VkFormat &image_format,
				 const VkColorSpaceKHR &image_colorspace,
				 const glm::u32vec2 &size,
				 std::uint32_t array_layers,
				 const VkSurfaceTransformFlagBitsKHR &transform,
				 VkCompositeAlphaFlagBitsKHR &composite_flags,
				 const VkPresentModeKHR &present_mode) : device(device) {
		VkSwapchainKHR swapchain;

		memset(&swapchain_create_info, 0, sizeof(swapchain_create_info));
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.pNext = nullptr;
		swapchain_create_info.flags = 0;
		swapchain_create_info.surface = surface;
		swapchain_create_info.minImageCount = min_image_count;
		swapchain_create_info.imageFormat = image_format;
		swapchain_create_info.imageColorSpace = image_colorspace;
		swapchain_create_info.imageExtent = { size.x, size.y };
		swapchain_create_info.imageArrayLayers = array_layers;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
		swapchain_create_info.preTransform = transform;
		swapchain_create_info.compositeAlpha = composite_flags;
		swapchain_create_info.presentMode = present_mode;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

		vk_result res = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
		if (!res) {
			throw vk_exception(res);
		}

		this->swapchain = swapchain;
	}
	~vk_swapchain() noexcept { destroy_swapchain(); }

	vk_swapchain(vk_swapchain &&) = default;
	vk_swapchain& operator=(vk_swapchain &&) = default;
	vk_swapchain(const vk_swapchain &) = delete;
	vk_swapchain& operator=(const vk_swapchain &) = delete;

	void resize(const glm::i32vec2 &size) {
		VkSwapchainCreateInfoKHR swapchain_info = swapchain_create_info;
		swapchain_info.imageExtent = { static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y) };
		swapchain_info.oldSwapchain = *this;

		VkSwapchainKHR swapchain;
		vk_result res = vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain);
		if (!res) {
			throw vk_exception(res);
		}

		this->swapchain_create_info = swapchain_info;
		this->swapchain = swapchain;
	}

	void destroy_swapchain() {
		if (swapchain) {
			vkDestroySwapchainKHR(device, swapchain.get(), nullptr);
			swapchain = none;
		}
	}

	auto& get_swapchain() const { return swapchain.get(); }
	auto& get_parameters() const { return swapchain_create_info; }

	operator VkSurfaceKHR() const { return get_swapchain(); }
};

}
}
