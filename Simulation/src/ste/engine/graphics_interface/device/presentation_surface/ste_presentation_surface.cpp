
#include <stdafx.hpp>
#include <vk_result.hpp>
#include <vk_swapchain.hpp>
#include <ste_presentation_surface.hpp>

#include <algorithm>
#include <functional>

using namespace ste::gl;

ste_presentation_surface::acquire_next_image_return_t  ste_presentation_surface::acquire_swapchain_image_impl(
	std::uint64_t timeout_ns,
	const vk::vk_semaphore *presentation_image_ready_semaphore,
	const vk::vk_fence *presentation_image_ready_fence) const
{
	acquire_next_image_return_t ret;
	vk::vk_result res;
	{
//		std::unique_lock<std::mutex> l(shared_data->swap_chain_guard);
		res = vkAcquireNextImageKHR(*presentation_device,
									*swap_chain,
									timeout_ns,
									presentation_image_ready_semaphore ? *presentation_image_ready_semaphore : VK_NULL_HANDLE,
									presentation_image_ready_fence ? *presentation_image_ready_fence : VK_NULL_HANDLE,
									&ret.image_index);
	}

	switch (res.get()) {
	case VK_SUBOPTIMAL_KHR:
		ret.sub_optimal = true;
	case VK_SUCCESS:
		ret.image = &swap_chain_images[ret.image_index];
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		ret.sub_optimal = true;
		break;
	default:
		throw vk::vk_exception(res);
	}

	// Furthermore raise flag to recreate swap-chain
	if (res != VK_SUCCESS)
		shared_data->swap_chain_optimal_flag.clear(std::memory_order_release);

	return ret;
}

void ste_presentation_surface::acquire_swap_chain_images() {
	auto format = this->swap_chain->get_format();
	auto layers = this->swap_chain->get_layers();
	auto size = this->swap_chain->get_extent();

	// Aquire swap-chain images
	std::vector<VkImage> swapchain_vk_image_objects;
	std::uint32_t chain_image_count;
	vk::vk_result res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, nullptr);
	if (!res) {
		this->swap_chain = nullptr;
		throw vk::vk_exception(res);
	}
	swapchain_vk_image_objects.resize(chain_image_count);
	res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, &swapchain_vk_image_objects[0]);
	if (!res) {
		this->swap_chain = nullptr;
		throw vk::vk_exception(res);
	}

	// Create views and image objects
	std::vector<swap_chain_image_t> images;
	images.reserve(swapchain_vk_image_objects.size());
	for (auto& img : swapchain_vk_image_objects) {
		auto image = vk::vk_swapchain_image(*presentation_device,
											img,
											format,
											vk::vk_swapchain_image::extent_type(size),
											layers);
		auto view = swap_chain_image_view_t(image);

		auto swapchain_image = device_swapchain_image(std::move(image));

		images.push_back({ std::move(swapchain_image), std::move(view) });
	}

	this->swap_chain_images = std::move(images);
}
glm::u32vec2 ste_presentation_surface::get_surface_extent() const {
	glm::u32vec2 extent = { surface_presentation_caps.currentExtent.width, surface_presentation_caps.currentExtent.height };
	glm::u32vec2 min_extent = { surface_presentation_caps.minImageExtent.width, surface_presentation_caps.minImageExtent.height };
	glm::u32vec2 max_extent = { surface_presentation_caps.maxImageExtent.width, surface_presentation_caps.maxImageExtent.height };

	return glm::clamp(extent, min_extent, max_extent);
}

VkPresentModeKHR ste_presentation_surface::get_surface_presentation_mode() const {
	static constexpr auto default_presentation_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	// Query available presentation modes
	std::uint32_t present_mode_count;
	std::vector<VkPresentModeKHR> supported_present_modes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(presentation_device->get_physical_device_descriptor().device,
											  presentation_surface, &present_mode_count, nullptr);
	supported_present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(presentation_device->get_physical_device_descriptor().device,
											  presentation_surface, &present_mode_count, &supported_present_modes[0]);

	if (!present_mode_count) {
		throw ste_device_exception("No supported presentation modes");
	}

	// Choose a presentation mode
	if (!parameters.vsync) {
		if (std::find(supported_present_modes.begin(),
					  supported_present_modes.end(),
					  default_presentation_mode) != supported_present_modes.end())
			return default_presentation_mode;
	}
	else {
		switch (parameters.vsync.get()) {
		case ste_presentation_device_vsync::mailbox:
			if (std::find(supported_present_modes.begin(),
						  supported_present_modes.end(),
						  VK_PRESENT_MODE_MAILBOX_KHR) != supported_present_modes.end())
				return VK_PRESENT_MODE_MAILBOX_KHR;
		case ste_presentation_device_vsync::fifo:
			if (std::find(supported_present_modes.begin(),
						  supported_present_modes.end(),
						  VK_PRESENT_MODE_FIFO_KHR) != supported_present_modes.end())
				return VK_PRESENT_MODE_FIFO_KHR;
		case ste_presentation_device_vsync::immediate:
		default:
			if (std::find(supported_present_modes.begin(),
						  supported_present_modes.end(),
						  VK_PRESENT_MODE_IMMEDIATE_KHR) != supported_present_modes.end())
				return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// None selected, return any available
	return supported_present_modes[0];
}

VkSurfaceFormatKHR ste_presentation_surface::get_surface_format() const {
	// Query available format
	std::uint32_t format_count;
	std::vector<VkSurfaceFormatKHR> supported_formats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(presentation_device->get_physical_device_descriptor().device,
										 presentation_surface, &format_count, nullptr);
	supported_formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(presentation_device->get_physical_device_descriptor().device,
										 presentation_surface, &format_count, &supported_formats[0]);

	if (!format_count) {
		throw ste_device_exception("No supported presentation image formats");
	}

	// Choose format
	auto rgb8_it = std::find_if(supported_formats.begin(),
								supported_formats.end(),
								[](const auto &surface_format) {
		return static_cast<format>(surface_format.format) == format::r8g8b8a8_unorm &&
			surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});
	auto bgr8_it = std::find_if(supported_formats.begin(),
								supported_formats.end(),
								[](const auto &surface_format) {
		return static_cast<format>(surface_format.format) == format::b8g8r8a8_unorm &&
			surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});
	auto any_srgb_it = std::find_if(supported_formats.begin(),
									supported_formats.end(),
									[](const auto &surface_format) {
		return surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	});

	if (rgb8_it != supported_formats.end())
		return *rgb8_it;
	if (bgr8_it != supported_formats.end())
		return *bgr8_it;
	if (any_srgb_it != supported_formats.end())
		return *any_srgb_it;

	// Return any supported format
	return supported_formats[0];
}

VkSurfaceTransformFlagBitsKHR ste_presentation_surface::get_transform() const {
	if (surface_presentation_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	if (surface_presentation_caps.supportedTransforms & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)
		return VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;

	throw ste_device_exception("Identity transform and inherit presentation transforms not supported");
}

void ste_presentation_surface::read_device_caps() {
	// Read device capabilities
	vk::vk_result res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentation_device->get_physical_device_descriptor().device,
																  presentation_surface,
																  &surface_presentation_caps);
	if (!res) {
		throw vk::vk_exception(res);
	}
}

void ste_presentation_surface::create_swap_chain() {
	// Create swap-chain based on passed parameters and available device capabilities
	if (!(surface_presentation_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
		throw ste_device_exception("VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT unsupported for swapchain");
	}

	// Swap chain properties
	auto size = get_surface_extent();
	std::uint32_t layers = 1;
	std::uint32_t min_image_count = surface_presentation_caps.minImageCount;
	std::uint32_t max_image_count = surface_presentation_caps.maxImageCount > 0 ?
		surface_presentation_caps.maxImageCount :
		std::numeric_limits<std::uint32_t>::max();

	// If parameters define a count of simultaneously acquired swap-chain images, override the default minimum
	// https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/vkspec.html#vkAcquireNextImageKHR
	// "vkAcquireNextImageKHR should not be called if a > n - m", where
	// n is the total number of images in the swapchain, 
	// m is the value of VkSurfaceCapabilitiesKHR::minImageCount, and 
	// a is the number of presentable images that the application has currently acquired.
	std::uint32_t desired_swap_chain_image_count = default_min_swap_chain_images;
	if (parameters.simultaneous_presentation_frames)
		desired_swap_chain_image_count = min_image_count + parameters.simultaneous_presentation_frames.get();

	auto swap_chain_image_count = glm::clamp<unsigned>(desired_swap_chain_image_count,
													   min_image_count,
													   max_image_count);
	auto format = get_surface_format();
	auto transform = get_transform();
	auto composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	auto present_mode = get_surface_presentation_mode();

	// Create the swap-chain
	this->swap_chain = std::make_unique<vk::vk_swapchain>(*presentation_device,
														  presentation_surface,
														  swap_chain_image_count,
														  format.format,
														  format.colorSpace,
														  size,
														  layers,
														  transform,
														  composite,
														  present_mode,
														  this->swap_chain.get());

	// And acquire the chain's presentation images
	acquire_swap_chain_images();
}

void ste_presentation_surface::connect_signals() {
	// Connect resize signal to window signals
	auto &resize_signal = presentation_window.get_signals().signal_window_resize();

	resize_signal_connection = std::make_shared<resize_signal_connection_t>([this](const glm::i32vec2 &size) {
		// Raise flag to recreate swap-chain
		shared_data->swap_chain_optimal_flag.clear(std::memory_order_release);
	});
	resize_signal.connect(resize_signal_connection);
}

void ste_presentation_surface::present(std::uint32_t image_index,
									   const vk::vk_queue &presentation_queue,
									   const semaphore &wait_semaphore) {
	VkSwapchainKHR swapchain = *swap_chain;

	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext = nullptr;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &wait_semaphore.get();
	info.swapchainCount = 1;
	info.pSwapchains = &swapchain;
	info.pImageIndices = &image_index;
	info.pResults = nullptr;

	vk::vk_result res;
	{
//		std::unique_lock<std::mutex> l(shared_data->swap_chain_guard);
		res = vkQueuePresentKHR(presentation_queue, &info);
	}

	// Raise flag to recreate swap-chain
	if (res != VK_SUCCESS)
		shared_data->swap_chain_optimal_flag.clear(std::memory_order_release);
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR && res != VK_ERROR_OUT_OF_DATE_KHR) {
		throw vk::vk_exception(res);
	}
}
