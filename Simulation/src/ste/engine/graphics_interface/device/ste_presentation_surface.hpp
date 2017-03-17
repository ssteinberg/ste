//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>

#include <ste_gl_context.hpp>
#include <vk_result.hpp>
#include <ste_device_exceptions.hpp>
#include <vk_logical_device.hpp>
#include <vk_swapchain.hpp>
#include <vk_swapchain_image.hpp>
#include <vk_image_view.hpp>
#include <vk_semaphore.hpp>
#include <vk_queue.hpp>

#include <connection.hpp>

#include <atomic>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <chrono>
#include <limits>
#include <mutex>

namespace StE {
namespace GL {

class ste_presentation_surface {
private:
	using resize_signal_connection_t = ste_window_signals::window_resize_signal_type::connection_type;

public:
	using swap_chain_image_view_t = vk_image_view<vk_image_type::image_2d>;
	struct swap_chain_image_t {
		vk_swapchain_image image;
		swap_chain_image_view_t view;

		swap_chain_image_t() = delete;
	};
	struct acquire_next_image_return_t {
		const swap_chain_image_t *image{ nullptr };
		std::uint32_t image_index{ 0 };
		bool sub_optimal{ false };
	};

private:
	const ste_gl_device_creation_parameters parameters;
	const vk_logical_device *presentation_device;
	const ste_window &presentation_window;

	vk_surface presentation_surface;
	VkSurfaceCapabilitiesKHR surface_presentation_caps;
	std::unique_ptr<vk_swapchain> swap_chain{ nullptr };
	std::vector<swap_chain_image_t> swap_chain_images;

	mutable std::mutex swap_chain_guard;

	std::shared_ptr<resize_signal_connection_t> resize_signal_connection;
	mutable std::atomic_flag swap_chain_optimal_flag = ATOMIC_FLAG_INIT;

private:
	auto get_surface_extent() const {
		glm::u32vec2 extent = { surface_presentation_caps.currentExtent.width, surface_presentation_caps.currentExtent.height };
		glm::u32vec2 min_extent = { surface_presentation_caps.minImageExtent.width, surface_presentation_caps.minImageExtent.height };
		glm::u32vec2 max_extent = { surface_presentation_caps.maxImageExtent.width, surface_presentation_caps.maxImageExtent.height };

		return glm::clamp(extent, min_extent, max_extent);
	}

	auto get_surface_presentation_mode() const {
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

	auto get_surface_format() const {
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
									[](const auto &format) {
			return format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		});
		auto bgr8_it = std::find_if(supported_formats.begin(),
									supported_formats.end(),
									[](const auto &format) {
			return format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		});
		auto any_srgb_it = std::find_if(supported_formats.begin(),
										supported_formats.end(),
										[](const auto &format) {
			return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
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

	auto get_transform() const {
		if (surface_presentation_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		if (surface_presentation_caps.supportedTransforms & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)
			return VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;

		throw ste_device_exception("Identity transform and inherit presentation transforms not supported");
	}

	void acquire_swap_chain_images() {
		auto format = this->swap_chain->get_format();
		auto layers = this->swap_chain->get_layers();
		auto size = this->swap_chain->get_size();

		// Aquire swap-chain images
		std::vector<VkImage> swapchain_vk_image_objects;
		std::uint32_t chain_image_count;
		vk_result res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, nullptr);
		if (!res) {
			this->swap_chain = nullptr;
			throw vk_exception(res);
		}
		swapchain_vk_image_objects.resize(chain_image_count);
		res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, &swapchain_vk_image_objects[0]);
		if (!res) {
			this->swap_chain = nullptr;
			throw vk_exception(res);
		}

		std::vector<swap_chain_image_t> images;
		images.reserve(swapchain_vk_image_objects.size());
		for (auto& img : swapchain_vk_image_objects) {
			auto image = vk_swapchain_image(*presentation_device,
											img,
											format,
											vk_swapchain_image::size_type(size),
											layers);
			auto view = swap_chain_image_view_t(image);

			images.push_back({ std::move(image), std::move(view) });
		}

		this->swap_chain_images = std::move(images);
	}

	void read_device_caps() {
		// Read device capabilities
		vk_result res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentation_device->get_physical_device_descriptor().device,
																  presentation_surface,
																  &surface_presentation_caps);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void create_swap_chain() {
		// Create swap-chain based on passed parameters and available device capabilities
		if (!(surface_presentation_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
			throw ste_device_exception("VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT unsupported for swapchain");
		}

		// Swap chain properties
		auto size = get_surface_extent();
		std::uint32_t layers = 1;
		std::uint32_t max_image_count = surface_presentation_caps.maxImageCount > 0 ?
			surface_presentation_caps.maxImageCount :
			std::numeric_limits<std::uint32_t>::max();
		auto min_image_count = glm::clamp<unsigned>(4,
													surface_presentation_caps.minImageCount,
													max_image_count);
		auto format = get_surface_format();
		auto transform = get_transform();
		auto composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		auto present_mode = get_surface_presentation_mode();

		// Create the swap-chain
		this->swap_chain = std::make_unique<vk_swapchain>(*presentation_device,
														  presentation_surface,
														  min_image_count,
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

	void connect_signals() {
		// Connect resize signal to window signals
		auto &resize_signal = presentation_window.get_signals().signal_window_resize();

		resize_signal_connection = std::make_shared<resize_signal_connection_t>([this](const glm::i32vec2 &size) {
			// Raise flag to recreate swap-chain
			swap_chain_optimal_flag.clear(std::memory_order_release);
		});
		resize_signal.connect(resize_signal_connection);
	}

private:
	auto acquire_swapchain_image_impl(std::uint64_t timeout_ns,
									  const vk_semaphore *presentation_image_ready_semaphore,
									  const vk_fence *presentation_image_ready_fence) const {
		acquire_next_image_return_t ret;
		vk_result res;
		{
			std::unique_lock<std::mutex> l(swap_chain_guard);
			res = vkAcquireNextImageKHR(presentation_device->get_device(),
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
			throw vk_exception(res);
		}

		// Furthermore raise flag to recreate swap-chain
		if (res != VK_SUCCESS)
			swap_chain_optimal_flag.clear(std::memory_order_release);

		return ret;
	}

public:
	/**
	*	@brief	Creates the presentation surface
	*
	*	@throws ste_device_exception	If creation parameters are erroneous or incompatible
	*	@throws ste_device_presentation_unsupported_exception	If presentation is not supported with supplied parameters
	*	@throws vk_exception	On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param presentation_device	The logical device that owns the surface
	*	@param presentation_window	Window to present to
	*	@param instance				Vulkan instance that owns the presentation device
	*/
	ste_presentation_surface(const ste_gl_device_creation_parameters parameters,
							 const vk_logical_device *presentation_device,
							 const ste_window &presentation_window,
							 const vk_instance &instance)
		: parameters(parameters),
		presentation_device(presentation_device),
		presentation_window(presentation_window),
		presentation_surface(presentation_window, instance)
	{
		assert(presentation_device && "Can not be null");

		// Check surface support
		bool has_present_support = false;
		for (unsigned i = 0; i < parameters.physical_device.queue_family_properties.size(); ++i) {
			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(parameters.physical_device.device, 0, presentation_surface, &supported);

			if ((has_present_support |= supported != 0))
				break;
		}
		if (!has_present_support) {
			throw ste_device_presentation_unsupported_exception("No device queues support presentation for choosen surface");
		}

		// Create swap-chain
		read_device_caps();
		create_swap_chain();

		// Connect required windowing system signals
		swap_chain_optimal_flag.test_and_set();
		connect_signals();
	}
	~ste_presentation_surface() noexcept {}

	ste_presentation_surface(ste_presentation_surface &&) = default;
	ste_presentation_surface &operator=(ste_presentation_surface &&) = default;
	ste_presentation_surface(const ste_presentation_surface &) = delete;
	ste_presentation_surface &operator=(const ste_presentation_surface &) = delete;

	/**
	*	@brief	Acquires the next swap-chain presentation image.
	*			Call result might be success, suboptimal, out-of-date, timeout or error.
	*			In case of success or suboptimal returns the next swap image.
	*			In case of suboptimal or out-of-date raises the 'sub_optimal' flag.
	*			In case of timeout or error, throws vk_exception.
	*
	*			Should be externally synchronized with other presentation methods.
	*
	*	@throws vk_exception	On timeout or Vulkan error
	*
	*	@param presentation_image_ready_semaphore	Semaphore to be signaled when returned image is ready to be drawn to.
	*	@param timeout								Timeout to wait for next image.
	*
	*	@return Returns a struct with a pointer to the pair swap_chain_image_t, index of the image and a 'sub_optimal' flag.
	*			The returned image might be nullptr.
	*			If the 'sub_optimal' flag is raised, the swap-chain should be recreated by calling recreate_swap_chain.
	*/
	template <class Rep = std::chrono::nanoseconds::rep, class Period = std::chrono::nanoseconds::period>
	acquire_next_image_return_t acquire_next_swapchain_image(
		const vk_semaphore &presentation_image_ready_semaphore,
		const std::chrono::duration<Rep, Period> &timeout = std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max())
	) const {
		std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
		return acquire_swapchain_image_impl(timeout_ns,
											&presentation_image_ready_semaphore,
											nullptr);
	}
	/**
	*	@brief	Acquires the next swap-chain presentation image.
	*			Call result might be success, suboptimal, out-of-date, timeout or error.
	*			In case of success or suboptimal returns the next swap image.
	*			In case of suboptimal or out-of-date raises the 'sub_optimal' flag.
	*			In case of timeout or error, throws vk_exception.
	*
	*			Should be externally synchronized with other presentation methods.
	*
	*	@throws vk_exception	On timeout or Vulkan error
	*
	*	@param presentation_image_ready_fence		Fence to be signaled when returned image is ready to be drawn to.
	*	@param timeout								Timeout to wait for next image.
	*
	*	@return Returns a struct with a pointer to the pair swap_chain_image_t, index of the image and a 'sub_optimal' flag.
	*			The returned image might be nullptr.
	*			If the 'sub_optimal' flag is raised, the swap-chain should be recreated by calling recreate_swap_chain.
	*/
	template <class Rep = std::chrono::nanoseconds::rep, class Period = std::chrono::nanoseconds::period>
	acquire_next_image_return_t acquire_next_swapchain_image(
		const vk_fence &presentation_image_ready_fence,
		const std::chrono::duration<Rep, Period> &timeout = std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max())
	) const {
		std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
		return acquire_swapchain_image_impl(timeout_ns,
											nullptr,
											&presentation_image_ready_fence);
	}

	/**
	*	@brief	Recreates the swap-chain. Possible following a surface resize or a suboptimial image.
	*			It is the callers responsibility to manually synchronize access to the old swap-chain.
	*
	*			Should be externally synchronized with other presentation methods.
	*
	*	@throws vk_exception	On Vulkan error during swap-chain recreation
	*	@throws ste_device_exception	On internal error during swap-chain recreation
	*/
	void recreate_swap_chain() {
		this->swap_chain_images.clear();
		read_device_caps();
		create_swap_chain();
	}

	/**
	*	@brief	Presents the presentation image specifided by the index.
	*
	*			Should be externally synchronized with other presentation methods.
	*
	*	@throws vk_exception	On Vulkan error
	*
	*	@param	image_index			Index of the presentation image in the swap-chain
	*	@param	presentation_queue	Device queue on which to present
	*	@param	wait_semaphore		Semaphore that signals that rendering to the presentation image is complete
	*/
	void present(std::uint32_t image_index,
				 const vk_queue &presentation_queue,
				 const vk_semaphore &wait_semaphore) {
		VkSemaphore semaphore = wait_semaphore;
		VkSwapchainKHR swapchain = *swap_chain;

		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &semaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &swapchain;
		info.pImageIndices = &image_index;
		info.pResults = nullptr;

		vk_result res;
		{
			std::unique_lock<std::mutex> l(swap_chain_guard);
			res = vkQueuePresentKHR(presentation_queue, &info);
		}

		// Raise flag to recreate swap-chain
		if (res != VK_SUCCESS)
			swap_chain_optimal_flag.clear(std::memory_order_release);
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR && res != VK_ERROR_OUT_OF_DATE_KHR) {
			throw vk_exception(res);
		}
	}

	bool test_and_clear_recreate_flag() const {
		return !swap_chain_optimal_flag.test_and_set(std::memory_order_acquire);
	}

	auto& get_swap_chain_images() const { return swap_chain_images; }
	auto& get_presentation_window() const { return presentation_window; }

	auto size() const { return swap_chain->get_size(); }
	auto format() const { return swap_chain->get_format(); }
	auto colorspace() const { return swap_chain->get_colorspace(); }
};

}
}
