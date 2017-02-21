//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>

#include <ste_gl_context.hpp>
#include <vk_result.hpp>
#include <ste_engine_exceptions.hpp>
#include <vk_logical_device.hpp>
#include <vk_swapchain.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#include <connection.hpp>

namespace StE {
namespace GL {

class ste_presentation_surface {
private:
	using resize_signal_connection_t = ste_window_signals::window_resize_signal_type::connection_type;

private:
	const ste_gl_presentation_device_creation_parameters parameters;
	const vk_logical_device *presentation_device;
	const ste_window &presentation_window;

	vk_surface presentation_surface;

	VkSurfaceCapabilitiesKHR surface_presentation_caps;
	std::unique_ptr<vk_swapchain> swap_chain{ nullptr };
	std::vector<VkImage> swap_chain_images;

	std::shared_ptr<resize_signal_connection_t> resize_signal_connection;

private:
	auto get_surface_extent() const {
		auto extent = presentation_window.get_framebuffer_size();
		glm::i32vec2 min_extent = { surface_presentation_caps.minImageExtent.width, surface_presentation_caps.minImageExtent.height };
		glm::i32vec2 max_extent = { surface_presentation_caps.maxImageExtent.width, surface_presentation_caps.maxImageExtent.height };

		return glm::clamp<glm::i32vec2>(extent, min_extent, max_extent);
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
			throw ste_engine_exception("No supported presentation modes");
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
			throw ste_engine_exception("No supported presentation image formats");
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

		throw ste_engine_exception("Identity and inherit presentation transforms not supported");
	}

	void setup_framebuffer() {
		// Create swap chain based on passed parameters and available device capabilities
		if (!(surface_presentation_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
			throw ste_engine_exception("VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT unsupported for swapchain");
		}
		
		auto size = get_surface_extent();
		auto min_image_count = glm::clamp<unsigned>(3,
													surface_presentation_caps.minImageCount,
													surface_presentation_caps.maxImageCount);
		auto format = get_surface_format();
		auto transform = get_transform();
		auto composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		auto present_mode = get_surface_presentation_mode();

		this->swap_chain = std::make_unique<vk_swapchain>(*presentation_device,
														  presentation_surface,
														  min_image_count,
														  format.format,
														  format.colorSpace,
														  size,
														  1,
														  transform,
														  composite,
														  present_mode);

		// Aquire swap chain images
		std::uint32_t chain_image_count;
		vk_result res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, nullptr);
		if (!res) {
			this->swap_chain = nullptr;
			throw vk_exception(res);
		}
		swap_chain_images.resize(chain_image_count);
		res = vkGetSwapchainImagesKHR(*presentation_device, *swap_chain, &chain_image_count, &swap_chain_images[0]);
		if (!res) {
			this->swap_chain = nullptr;
			throw vk_exception(res);
		}
	}

	void resize_framebuffer(const glm::i32vec2 &size) {
		this->swap_chain->resize(size);
	}

	void setup_signals() {
		// Connect resize signal to window signals
		auto &resize_signal = presentation_window.get_signals().signal_window_resize();

		resize_signal_connection = std::make_shared<resize_signal_connection_t>([this](const glm::i32vec2 &size) {
			// Recreate swap chain
			this->resize_framebuffer(size);
		});
		resize_signal.connect(resize_signal_connection);
	}

public:
	ste_presentation_surface(const ste_gl_presentation_device_creation_parameters parameters,
							 const vk_logical_device *presentation_device,
							 const ste_window &presentation_window,
							 const vk_instance &instance)
		: parameters(parameters),
		presentation_device(presentation_device),
		presentation_window(presentation_window),
		presentation_surface(presentation_window, instance)
	{
		assert(presentation_device && "Can not be null");

		// Read device capabilities
		vk_result res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(presentation_device->get_physical_device_descriptor().device,
																  presentation_surface,
																  &surface_presentation_caps);
		if (!res) {
			throw vk_exception(res);
		}

		// Check surface support
		bool has_present_support = false;
		for (unsigned i = 0; i < parameters.physical_device.queue_family_properties.size(); ++i) {
			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(parameters.physical_device.device, 0, presentation_surface, &supported);

			if ((has_present_support |= supported != 0))
				break;
		}
		if (!has_present_support) {
			throw ste_engine_exception("No device queues support presentation for choosen surface");
		}

		// Create swap chain and connect signals
		setup_framebuffer();
		setup_signals();
	}
};

}
}
