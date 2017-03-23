//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
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
	std::reference_wrapper<const vk_logical_device> device;

public:
	/**
	*	@brief	Creates a swap-chain.
	*
	*	@param device		The device that owns the swap-chain
	*	@param surface		Surface on which the swap-chain will be used for presentation
	*	@param min_image_count	Minimal requested swap-chain images
	*	@param image_format	Images' format
	*	@param image_colorspace	Images' colorspace
	*	@param size			Images' size, should match surface extent
	*	@param array_layers	Images' layers
	*	@param transform	Specifies how images should be transformed prior to presentation
	*	@param composite_flags	Composition flags
	*	@param present_mode	Presentation mode
	*	@param old_chain	If non-nullptr, the new chain will be recreated from old_chain, at which point 'old_chain'
	*						will be moved from and no longer a valid swap-chain object
	*
	*	@return Returns a struct with a pointer to the pair swap_chain_image_t and a 'sub_optimal' flag.
	*/
	vk_swapchain(const vk_logical_device &device,
				 const vk_surface &surface,
				 std::uint32_t min_image_count,
				 const VkFormat &image_format,
				 const VkColorSpaceKHR &image_colorspace,
				 const glm::u32vec2 &size,
				 std::uint32_t array_layers,
				 const VkSurfaceTransformFlagBitsKHR &transform,
				 VkCompositeAlphaFlagBitsKHR &composite_flags,
				 const VkPresentModeKHR &present_mode,
				 vk_swapchain *old_chain = nullptr) : device(device) {
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
		swapchain_create_info.oldSwapchain = old_chain ? *old_chain : VK_NULL_HANDLE;

		vk_result res = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
		if (!res) {
			throw vk_exception(res);
		}

		this->swapchain = swapchain;
		swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
	}
	~vk_swapchain() noexcept { destroy_swapchain(); }

	vk_swapchain(vk_swapchain &&) = default;
	vk_swapchain& operator=(vk_swapchain &&) = default;
	vk_swapchain(const vk_swapchain &) = delete;
	vk_swapchain& operator=(const vk_swapchain &) = delete;

	void destroy_swapchain() {
		if (swapchain) {
			vkDestroySwapchainKHR(device.get(), swapchain.get(), nullptr);
			swapchain = none;
		}
	}

	auto& get_swapchain() const { return swapchain.get(); }
	auto get_format() const { return swapchain_create_info.imageFormat; }
	auto get_colorspace() const { return swapchain_create_info.imageColorSpace; }
	auto get_present_mode() const { return swapchain_create_info.presentMode; }
	auto get_layers() const { return swapchain_create_info.imageArrayLayers; }
	auto get_size() const {
		return glm::u32vec2{ swapchain_create_info.imageExtent.width, swapchain_create_info.imageExtent.height };
	}
	auto get_creation_parameters() const { return swapchain_create_info; }

	operator VkSurfaceKHR() const { return get_swapchain(); }
};

}
}
