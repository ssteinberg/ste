//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <ste_gl_context_creation_parameters.hpp>

#include <ste_gl_context.hpp>
#include <vk_surface.hpp>
#include <ste_device_exceptions.hpp>
#include <vk_logical_device.hpp>
#include <vk_swapchain.hpp>
#include <device_swapchain_image.hpp>
#include <image_view.hpp>
#include <vk_queue.hpp>
#include <semaphore.hpp>

#include <format.hpp>
#include <colorspace.hpp>

#include <atomic>
#include <memory>
#include <vector>
#include <chrono>
#include <limits>
#include <mutex>
#include <aligned_ptr.hpp>

namespace ste {
namespace gl {

class ste_presentation_surface {
public:
	static constexpr std::uint32_t default_min_swap_chain_images = 3;

	using swap_chain_image_view_t = image_view<image_type::image_2d>;
	struct swap_chain_image_t {
		device_swapchain_image image;
		swap_chain_image_view_t view;

		swap_chain_image_t() = delete;
	};
	struct acquire_next_image_return_t {
		const swap_chain_image_t *image{ nullptr };
		std::uint32_t image_index{ 0 };
		bool sub_optimal{ false };
	};

	struct shared_data_t {
		mutable std::mutex swap_chain_guard;
		mutable std::atomic_flag swap_chain_optimal_flag = ATOMIC_FLAG_INIT;
	};

private:
	const ste_gl_device_creation_parameters parameters;
	const vk::vk_logical_device *presentation_device;
	const ste_window &presentation_window;

	vk::vk_surface presentation_surface;
	VkSurfaceCapabilitiesKHR surface_presentation_caps;
	std::unique_ptr<vk::vk_swapchain> swap_chain{ nullptr };
	std::vector<swap_chain_image_t> swap_chain_images;

	aligned_ptr<shared_data_t> shared_data;

	ste_window_signals::window_resize_signal_type::connection_type resize_signal_connection;

private:
	glm::u32vec2 get_surface_extent() const;
	VkPresentModeKHR get_surface_presentation_mode() const;
	VkSurfaceFormatKHR get_surface_format() const;
	VkSurfaceTransformFlagBitsKHR get_transform() const;
	void acquire_swap_chain_images();
	void read_device_caps();
	void create_swap_chain();
	void connect_signals();

private:
	acquire_next_image_return_t acquire_swapchain_image_impl(std::uint64_t timeout_ns,
															 const vk::vk_semaphore *presentation_image_ready_semaphore,
															 const vk::vk_fence *presentation_image_ready_fence) const;

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
							 const vk::vk_logical_device *presentation_device,
							 const ste_window &presentation_window,
							 const vk::vk_instance &instance)
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
		shared_data->swap_chain_optimal_flag.test_and_set();
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
		const vk::vk_semaphore &presentation_image_ready_semaphore,
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
		const vk::vk_fence &presentation_image_ready_fence,
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
				 const vk::vk_queue &presentation_queue,
				 const semaphore &wait_semaphore);

	bool test_and_clear_recreate_flag() const {
		return !shared_data->swap_chain_optimal_flag.test_and_set(std::memory_order_acquire);
	}

	/**
	 *	@brief	Returns swap chain images count
	 */
	auto& get_swap_chain_images() const { return swap_chain_images; }
	/**
	*	@brief	Returns the maximal count of acquired swap chain images at any given time.
	*			See Vulkan specifications: https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/vkspec.html#vkAcquireNextImageKHR
	*/
	auto get_max_allowed_acquired_swap_chain_images() const {
		auto images = static_cast<std::uint32_t>(get_swap_chain_images().size());
		auto min_surface_images = surface_presentation_caps.minImageCount;

		return images - min_surface_images + 1;
	}

	auto& get_presentation_window() const { return presentation_window; }

	auto extent() const { return swap_chain->get_extent(); }
	auto surface_format() const { return static_cast<format>(swap_chain->get_format()); }
	auto surface_colorspace() const { return static_cast<colorspace>(swap_chain->get_colorspace()); }
};

}
}
