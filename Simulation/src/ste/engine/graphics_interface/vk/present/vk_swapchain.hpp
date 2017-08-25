//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_handle.hpp>

#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_surface.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_swapchain : public allow_type_decay<vk_swapchain<host_allocator>, VkSwapchainKHR> {
private:
	VkSwapchainCreateInfoKHR swapchain_create_info;
	optional<VkSwapchainKHR> swapchain;
	alias<const vk_logical_device<host_allocator>> device;

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
	vk_swapchain(const vk_logical_device<host_allocator> &device,
				 const vk_surface<host_allocator> &surface,
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
		swapchain_create_info.oldSwapchain = old_chain ? 
			static_cast<VkSwapchainKHR>(*old_chain) : 
			vk_null_handle;

		vk_result res = vkCreateSwapchainKHR(device, &swapchain_create_info, &host_allocator::allocation_callbacks(), &swapchain);
		if (!res) {
			throw vk_exception(res);
		}

		this->swapchain = swapchain;
		swapchain_create_info.oldSwapchain = vk_null_handle;
	}
	~vk_swapchain() noexcept { destroy_swapchain(); }

	vk_swapchain(vk_swapchain &&) = default;
	vk_swapchain& operator=(vk_swapchain &&o) noexcept {
		destroy_swapchain();

		swapchain = std::move(o.swapchain);
		device = std::move(o.device);
		swapchain_create_info = std::move(o.swapchain_create_info);

		return *this;
	}
	vk_swapchain(const vk_swapchain &) = delete;
	vk_swapchain& operator=(const vk_swapchain &) = delete;

	void destroy_swapchain() {
		if (swapchain) {
			vkDestroySwapchainKHR(device.get(), swapchain.get(), &host_allocator::allocation_callbacks());
			swapchain = none;
		}
	}

	auto& get() const { return swapchain.get(); }
	auto get_format() const { return swapchain_create_info.imageFormat; }
	auto get_colorspace() const { return swapchain_create_info.imageColorSpace; }
	auto get_present_mode() const { return swapchain_create_info.presentMode; }
	auto get_layers() const { return swapchain_create_info.imageArrayLayers; }
	auto get_extent() const {
		return glm::u32vec2{ swapchain_create_info.imageExtent.width, swapchain_create_info.imageExtent.height };
	}
	auto get_creation_parameters() const { return swapchain_create_info; }
};

}

}
}
