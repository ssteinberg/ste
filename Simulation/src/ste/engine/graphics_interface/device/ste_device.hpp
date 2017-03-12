//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <ste_engine_exceptions.hpp>
#include <ste_gl_context_creation_parameters.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_gl_device_queue.hpp>

#include <signal.hpp>

#include <memory>
#include <vector>
#include <functional>
#include <function_traits.hpp>

namespace StE {
namespace GL {

class ste_device {
public:
	using queues_and_surface_recreate_signal_type = signal<const ste_device*>;

private:
	using queue_t = std::unique_ptr<ste_gl_device_queue>;

private:
	const ste_gl_device_creation_parameters parameters;
	const std::vector<ste_gl_queue_descriptor> queue_descriptors;
	vk_logical_device device;
	const std::unique_ptr<ste_presentation_surface> presentation_surface{ nullptr };

	std::uint32_t command_buffers_count{ 0 };
	std::vector<queue_t> device_queues;

	queues_and_surface_recreate_signal_type queues_and_surface_recreate_signal;

private:
	static thread_local ste_presentation_surface::acquire_next_image_return_t acquired_presentation_image;

public:
	static auto &next_presentation_image() { return acquired_presentation_image; }

private:
	static auto create_vk_virtual_device(const GL::vk_physical_device_descriptor &physical_device,
										 const VkPhysicalDeviceFeatures &requested_features,
										 const std::vector<ste_gl_queue_descriptor> &queue_descriptors,
										 std::vector<const char*> device_extensions = {}) {
		if (queue_descriptors.size() == 0) {
			throw ste_engine_exception("queue_descriptors is empty");
		}

		// Add required extensions
		device_extensions.push_back("VK_KHR_swapchain");

		// Request queues based on supplied protocol
		std::vector<VkDeviceQueueCreateInfo> queues_create_infos;
		queues_create_infos.resize(queue_descriptors.size());
		for (int i = 0; i < queue_descriptors.size(); ++i)
			queues_create_infos[i] = queue_descriptors[i].create_device_queue_create_info();

		// Create logical device
		GL::vk_logical_device device(physical_device,
									 requested_features,
									 queues_create_infos,
									 device_extensions);

		return device;
	}

	//	void setup_queues_indices(const std::vector<ste_gl_queue_descriptor> &queues) {
	//		main_queue_idx =
	//			std::find_if(queues.begin(), queues.end(),
	//						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::main_queue; }) - queues.begin();
	//		if (main_queue_idx == queues.size())
	//			throw ste_engine_exception("No main queue specified in queue descriptors.");
	//
	//		compute_queue_idx =
	//			std::find_if(queues.begin(), queues.end(),
	//						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::compute_queue; }) - queues.begin();
	//		transfer_queue_idx =
	//			std::find_if(queues.begin(), queues.end(),
	//						 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::data_transfer_queue; }) - queues.begin();
	//
	//		if (compute_queue_idx == queues.size())
	//			compute_queue_idx = std::find_if(queues.begin(), queues.end(),
	//											 [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::main_queue; }) - queues.begin();
	//		if (transfer_queue_idx == queues.size())
	//			transfer_queue_idx = std::find_if(queues.begin(), queues.end(),
	//											  [](const ste_gl_queue_descriptor &q) { return q.usage == ste_gl_queue_usage::main_queue; }) - queues.begin();
	//	}

	void create_queues() {
		if (presentation_surface != nullptr) {
			// For devices with a presentation surface, command buffers count is exactly the amount of swap chain images.
			// For compute only devices, command buffers count is selected by the consumer.
			command_buffers_count = presentation_surface->get_swap_chain_images().size();
			assert(command_buffers_count);
		}

		std::vector<queue_t> q;
		q.reserve(queue_descriptors.size());
		for (int idx=0; idx<queue_descriptors.size(); ++idx)
			q.push_back(std::make_unique<queue_t::element_type>(device,
																queue_descriptors[idx],
																get_command_buffers_count(),
																idx));

		device_queues = std::move(q);
	}

	void recreate_swap_chain() {
		// Destroy queues, this will wait for all queue threads and then wait for all queues to finish processing,
		// allowing us to recreate the swap chain and queue safely.
		device_queues.clear();
		device.wait_idle();

		// Recreate swap chain
		presentation_surface->recreate_swap_chain();
		// And queues
		create_queues();

		// Signal
		queues_and_surface_recreate_signal.emit(this);
	}

public:
	/**
	*	@brief	Creates the device with presentation surface and capabilities
	*
	*	@throws ste_engine_exception	If creation parameters are erroneous or incompatible
	*	@throws vk_exception	On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param gl_ctx				Context
	*	@param presentation_window	Presentation window used for rendering
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const std::vector<ste_gl_queue_descriptor> &queue_descriptors,
			   const ste_gl_context &gl_ctx,
			   const ste_window &presentation_window)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		presentation_surface(std::make_unique<ste_presentation_surface>(parameters,
																		&device,
																		presentation_window,
																		gl_ctx.instance()))
	{
		// Create queues
		create_queues();
	}
	/**
	*	@brief	Creates the device without presentation capabilities ("compute-only" device)
	*
	*	@throws ste_engine_exception	If creation parameters are erroneous or incompatible
	*	@throws vk_exception	On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param gl_ctx				Context
	*	@param command_buffers_count Count of command buffers to create in each queue
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const std::vector<ste_gl_queue_descriptor> &queue_descriptors,
			   const ste_gl_context &gl_ctx,
			   std::uint32_t command_buffers_count = 1)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		command_buffers_count(command_buffers_count)
	{
		if (command_buffers_count == 0) {
			throw ste_engine_exception("command_buffers_count is zero");
		}

		// Create queues
		create_queues();
	}
	~ste_device() noexcept {}

	ste_device(ste_device &&) = default;
	ste_device &operator=(ste_device &&) = default;

	/**
	*	@brief	Prepares the next command buffer
	*
	*	@throws vk_exception	On Vulkan error
	*	
	*	@param queue_idx		The device queue index to use for presenatation
	*/
	void acquire_next_command_buffer(int queue_idx) {
		const auto& queue = device_queues[queue_idx];
		queue->acquire_next_command_buffer();
	}

	/**
	*	@brief	Enqueues a task on the main queue to acquire next presentation image and resize/recreate swap chain if necessary.
	*			The task will save the presentation image information in the thread_local variable, which will be consumed on
	*			next call to present.
	*
	*			Only available for presentation-capable devices.
	*
	*	@param queue_idx		The device queue index to use for presenatation
	*	@param presentation_image_ready_semaphore	Semaphore to be signaled when next presentation image is ready to be drawn to.
	*/
	void acquire_presentation_image(int queue_idx,
									const vk_semaphore &presentation_image_ready_semaphore) const {
		const auto& queue = device_queues[queue_idx];
		queue->enqueue<void>([this, &presentation_image_ready_semaphore](std::uint32_t index) {
			// Acquire next presenation image
			acquired_presentation_image = presentation_surface->acquire_next_swapchain_image(presentation_image_ready_semaphore);
		});
	}

	/**
	*	@brief	Enqueues a task on the queue's thread
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	auto enqueue(int queue_idx, L &&lambda) {
		using lambda_result_t = typename function_traits<L>::result_t;

		const auto& queue = device_queues[queue_idx];
		return queue->enqueue<lambda_result_t>(std::forward<L>(lambda));
	}

	/**
	*	@brief	Enqueues a submit task that submits the acquired command buffer to the queue
	*
	*	@param queue_idx		The device queue index to use for presenatation
	*	@param wait_semaphores		See vk_queue::submit
	*	@param signal_semaphores	See vk_queue::submit
	*/
	auto submit(int queue_idx,
				const std::vector<std::pair<vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
				const std::vector<vk_semaphore*> &signal_semaphores) {
		const auto& queue = device_queues[queue_idx];
		return queue->submit(wait_semaphores,
							 signal_semaphores);
	}

	/**
	*	@brief	Presents the presentation image.
	*			Might stall if swap chain recreation is required.
	*
	*			Only available for presentation-capable devices.
	*
	*	@throws vk_exception	On Vulkan error during swap chain recreation
	*	@throws ste_engine_exception	On internal error during swap chain recreation
	*
	*	@param queue_idx		The device queue index to use for presenatation
	*	@param rendering_ready_semaphore	Semaphore to be signaled when rendering to the presentation image is complete.
	*/
	void present(int queue_idx,
				 const vk_semaphore &rendering_ready_semaphore) {
		this->enqueue(queue_idx, [this, &rendering_ready_semaphore](std::uint32_t index) {
			if (acquired_presentation_image.image != nullptr) {
				this->presentation_surface->present(acquired_presentation_image.image_index,
													ste_gl_device_queue::thread_queue(),
													rendering_ready_semaphore);
				acquired_presentation_image = {};
			}
		});

		if (presentation_surface->test_and_clear_recreate_flag()) {
			// Recreate swap chain and queues
			recreate_swap_chain();
		}
	}

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_surface() const { return *presentation_surface; }
	auto& get_queue(int idx) const { return *device_queues[idx]; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_command_buffers_count() const { return command_buffers_count; }

	auto& logical_device() const { return device; }
};

}
}
