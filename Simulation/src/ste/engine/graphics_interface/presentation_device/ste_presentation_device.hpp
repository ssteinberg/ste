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

namespace StE {
namespace GL {

class ste_presentation_device {
public:
	using queues_and_surface_recreate_signal_type = signal<const ste_presentation_device*>;

private:
	using queue_t = std::unique_ptr<ste_gl_device_queue>;

private:
	const ste_gl_presentation_device_creation_parameters parameters;
	const std::vector<ste_gl_queue_descriptor> queue_descriptors;
	vk_logical_device presentation_device;
	ste_presentation_surface presentation_surface;

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
		std::vector<queue_t> q;
		q.reserve(queue_descriptors.size());
		for (auto &d : queue_descriptors)
			q.push_back(std::make_unique<queue_t::element_type>(presentation_device, 
																d, 
																get_command_buffers_count()));

		device_queues = std::move(q);
	}

	void recreate_swap_chain() {
		// Destroy queues, this will wait for all queue threads and then wait for all queues to finish processing,
		// allowing us to recreate the swap chain and queue safely.
		device_queues.clear();

		// Recreate swap chain
		presentation_surface.recreate_swap_chain();
		// And queues
		create_queues();

		// Signal
		queues_and_surface_recreate_signal.emit(this);
	}

public:
	/**
	*	@brief	Creates the presentation device.
	*
	*	@param parameters		Device creation parameters
	*/
	ste_presentation_device(const ste_gl_presentation_device_creation_parameters &parameters,
							const std::vector<ste_gl_queue_descriptor> &queue_descriptors,
							const ste_gl_context &gl_ctx,
							const ste_window &presentation_window)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		presentation_device(create_vk_virtual_device(parameters.physical_device,
													 parameters.requested_device_features,
													 queue_descriptors,
													 parameters.additional_device_extensions)),
		presentation_surface(parameters, &presentation_device, presentation_window, gl_ctx.instance())
	{
		// Create queues
		create_queues();
	}
	~ste_presentation_device() noexcept {}

	ste_presentation_device(ste_presentation_device &&) = default;
	ste_presentation_device &operator=(ste_presentation_device &&) = default;

	/**
	*	@brief	Polls windowing system events. Will resize/recreate swap chain if necessary.
	*			Might stall till a command buffer becomes available.
	*/
	void tick() {
		glfwPollEvents();

		bool recreate = presentation_surface.test_and_clear_recreate_flag();
		if (recreate) {
			// Recreate swap chain and queues
			recreate_swap_chain();
		}
	}

	/**
	*	@brief	Enqueues a task on the main queue to acquire next presentation image and resize/recreate swap chain if necessary.
	*			The task will save the presentation image information in the thread_local variable, which will be consumed on 
	*			next call to present.
	*
	*	@param queue						The queue to use for presenatation
	*	@param presentation_image_ready_semaphore	Semaphore to be signaled when next presentation image is ready to be drawn to.
	*/
	void acquire_presentation_image(const queue_t &queue,
									const vk_semaphore &presentation_image_ready_semaphore) {
		queue->enqueue<void>([this, &presentation_image_ready_semaphore](std::uint32_t index) {
			// Acquire next presenation image
			acquired_presentation_image = presentation_surface.acquire_next_swapchain_image(presentation_image_ready_semaphore);
		});
	}

	/**
	*	@brief	Presents the presentation image.
	*
	*	@param queue						The queue to use for presenatation
	*	@param rendering_ready_semaphore	Semaphore to be signaled when rendering to the presentation image is complete.
	*/
	void present(const queue_t &queue,
				 const vk_semaphore &rendering_ready_semaphore) {
		queue->enqueue<void>([this, &queue, &rendering_ready_semaphore](std::uint32_t index) {
			if (acquired_presentation_image.image != nullptr) {
				this->presentation_surface.present(acquired_presentation_image.image_index,
												   queue->device_queue(),
												   rendering_ready_semaphore);
				acquired_presentation_image = {};
			}
		});
	}

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_queue(int idx) const { return *device_queues[idx]; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_command_buffers_count() const { return presentation_surface.swap_chain_images_count(); }

	auto& device() const { return presentation_device; }
};

}
}
