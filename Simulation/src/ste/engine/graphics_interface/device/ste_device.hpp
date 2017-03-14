//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_device_queue_selector_cache.hpp>
#include <ste_queue_selector.hpp>

#include <ste_engine_exceptions.hpp>
#include <ste_gl_context_creation_parameters.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_device_queue.hpp>

#include <vk_semaphore.hpp>

#include <signal.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace StE {
namespace GL {

class ste_device {
public:
	using queues_and_surface_recreate_signal_type = signal<const ste_device*>;

private:
	using queue_t = std::unique_ptr<ste_device_queue>;
	struct presentation_sync_primitives_t {
		vk_semaphore swapchain_image_ready_semaphore;
		vk_semaphore rendering_finished_semaphore;

		presentation_sync_primitives_t() = delete;
		presentation_sync_primitives_t(const vk_logical_device &device)
			: swapchain_image_ready_semaphore(device), rendering_finished_semaphore(device)
		{}
		presentation_sync_primitives_t(presentation_sync_primitives_t &&) = default;
		presentation_sync_primitives_t &operator=(presentation_sync_primitives_t &&) = default;
	};
	struct presentation_next_image_t {
		ste_presentation_surface::acquire_next_image_return_t next_image;
		const presentation_sync_primitives_t *sync{ nullptr };
	};

private:
	const ste_gl_device_creation_parameters parameters;
	const ste_queue_descriptors queue_descriptors;
	const vk_logical_device device;
	const std::unique_ptr<ste_presentation_surface> presentation_surface{ nullptr };

	const std::uint32_t minimal_command_buffers_count{ 0 };
	std::uint32_t command_buffers_count{ 0 };
	std::vector<presentation_sync_primitives_t> presentation_sync_primitives;

	ste_device_queue_selector_cache queue_selector_cache;
	std::vector<queue_t> device_queues;
	
	queues_and_surface_recreate_signal_type queues_and_surface_recreate_signal;

private:
	static thread_local presentation_next_image_t acquired_presentation_image;

public:
	static auto &next_presentation_image() { return acquired_presentation_image.next_image; }

private:
	static auto create_vk_virtual_device(const GL::vk_physical_device_descriptor &physical_device,
										 const VkPhysicalDeviceFeatures &requested_features,
										 const ste_queue_descriptors &queue_descriptors,
										 std::vector<const char*> device_extensions = {}) {
		if (queue_descriptors.size() == 0) {
			throw ste_engine_exception("queue_descriptors is empty");
		}

		// Add required extensions
		device_extensions.push_back("VK_KHR_swapchain");

		// Request queues based on supplied protocol
		auto queues_create_info = queue_descriptors.create_device_queue_create_info();

		// Create logical device
		return GL::vk_logical_device(physical_device,
									 requested_features,
									 queues_create_info->create_info,
									 device_extensions);
	}

	void create_queues() {
		std::vector<queue_t> q;
		q.reserve(queue_descriptors.size());
		
		std::uint32_t prev_family = 0xFFFFFFFF;
		std::uint32_t family_index = 0;
		for (int idx = 0; idx < queue_descriptors.size(); ++idx) {
			auto &descriptor = queue_descriptors[idx];

			// Family index is used to get the queue from Vulkan
			descriptor.family == prev_family ? 
				++family_index :
				(family_index = 0);
			prev_family = descriptor.family;

			// Create the device queue
			q.push_back(std::make_unique<queue_t::element_type>(device,
																family_index,
																descriptor,
																get_command_buffers_count(),
																idx));
		}

		device_queues = std::move(q);

		// And reset the queue selector cache
		queue_selector_cache = ste_device_queue_selector_cache();
	}

	void update_command_buffers_count() {
		// For devices with a presentation surface: Try to create a command buffer per swap-chain image
		// For compute only devices, command buffers count is static.
		command_buffers_count = std::max(presentation_surface->get_swap_chain_images().size(),
										 minimal_command_buffers_count);
		assert(command_buffers_count);

		// Create synchronization primitives
		std::vector<presentation_sync_primitives_t> v;
		v.reserve(command_buffers_count);
		for (int i=0; i<command_buffers_count; ++i)
			v.emplace_back(device);
		presentation_sync_primitives = std::move(v);
	}

	void recreate_swap_chain() {
		// Destroy queues, this will wait for all queue threads and then wait for all queues to finish processing,
		// allowing us to recreate the swap-chain and queue safely.
		device_queues.clear();
		device.wait_idle();
		// Free synchronization primitives
		presentation_sync_primitives.clear();

		// Recreate swap-chain
		presentation_surface->recreate_swap_chain();

		// Update command buffers count based on new swap-chain
		// This will also recreate the synchronization primitives
		update_command_buffers_count();
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
	*	@param minimal_command_buffers_count Minimal amount of command buffers to create
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
			   const ste_gl_context &gl_ctx,
			   const ste_window &presentation_window,
			   std::uint32_t minimal_command_buffers_count = 1)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		presentation_surface(std::make_unique<ste_presentation_surface>(parameters,
																		&device,
																		presentation_window,
																		gl_ctx.instance())),
		minimal_command_buffers_count(minimal_command_buffers_count)
	{
		// Choose command buffers count based on the created swap-chain
		update_command_buffers_count();
		// Create queues
		create_queues();
	}
	/**
	*	@brief	Creates the device without presentation capabilities ("compute-only" device)
	*
	*	@throws ste_engine_exception	If creation parameters are erroneous or incompatible
	*	@throws vk_exception		On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param gl_ctx				Context
	*	@param command_buffers_count Count of command buffers to create in each queue
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
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
	*	@throws ste_engine_exception	If no compatible queue can be found
	*	
	*	@param queue_selector		The device queue selector used to select the device queue to use
	*/
	template <typename selector_policy = ste_queue_selector_default_policy>
	void acquire_next_command_buffer(const ste_queue_selector<selector_policy> &queue_selector) {
		auto& queue = select_queue(queue_selector);
		queue->acquire_next_command_buffer();
	}

	/**
	*	@brief	Enqueues a task on the main queue to acquire next presentation image and resize/recreate swap-chain if necessary.
	*			The task will save the presentation image information in the thread_local variable, which will be consumed on
	*			next call to present.
	*
	*			Only available for presentation-capable devices.
	*			
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param queue_selector		The device queue selector used to select the device queue to use
	*/
	template <typename selector_policy = ste_queue_selector_default_policy>
	void acquire_presentation_image(const ste_queue_selector<selector_policy> &queue_selector) {
		auto& queue = select_queue(queue_selector);

		queue->enqueue([this](std::uint32_t index) {
			const auto* sync = &presentation_sync_primitives[index];

			assert(acquired_presentation_image.sync == nullptr && "Last acquired image was not presented!");

			// Acquire next presenation image
			presentation_next_image_t next_image_descriptor;
			next_image_descriptor.next_image = presentation_surface->acquire_next_swapchain_image(sync->swapchain_image_ready_semaphore);
			next_image_descriptor.sync = sync;

			acquired_presentation_image = next_image_descriptor;
		});
	}

	/**
	*	@brief	Enqueues a task on the queue's thread
	*	
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param	queue_selector		The device queue selector used to select the device queue to use
	*	@param	task	Task to enqueue
	*/
	template <typename L, typename selector_policy = ste_queue_selector_default_policy>
	auto enqueue(const ste_queue_selector<selector_policy> &queue_selector, L &&task) {
		auto& queue = select_queue(queue_selector);
		return queue->enqueue(std::forward<L>(task));
	}

	/**
	*	@brief	Enqueues a submit task that submits the acquired command buffer to the queue
	*	
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param queue_selector		The device queue selector used to select the device queue to use
	*	@param wait_semaphores		See vk_queue::submit
	*	@param signal_semaphores	See vk_queue::submit
	*/
	template <typename selector_policy = ste_queue_selector_default_policy>
	auto submit(const ste_queue_selector<selector_policy> &queue_selector,
				const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
				const std::vector<const vk_semaphore*> &signal_semaphores) {
		return this->enqueue(queue_selector, [=](std::uint32_t index) {
			ste_device_queue::submit_current_queue(index,
													  wait_semaphores,
													  signal_semaphores);
		});
	}

	/**
	*	@brief	Enqueues a submission and presentation task that submits the acquired command buffer
	*			and then presents the acquired presentation image.
	*			Might stall if swap-chain recreation is required.
	*
	*			Only available for presentation-capable devices.
	*
	*	@throws vk_exception	On Vulkan error during swap-chain recreation
	*	@throws ste_engine_exception	On internal error during swap-chain recreation
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param queue_selector		The device queue selector used to select the device queue to use
	*	@param wait_semaphores		See vk_queue::submit
	*	@param signal_semaphores	See vk_queue::submit
	*/
	template <typename selector_policy = ste_queue_selector_default_policy>
	void submit_and_present(const ste_queue_selector<selector_policy> &queue_selector,
							const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
							const std::vector<const vk_semaphore*> &signal_semaphores) {
		this->enqueue(queue_selector, [this, signal_semaphores, wait_semaphores](std::uint32_t index) mutable {
			const auto* sync = acquired_presentation_image.sync;
			assert(acquired_presentation_image.sync != nullptr && "Present called without acquire!");

			// Synchronize submission with presentation
			auto wait = wait_semaphores;
			auto signal = signal_semaphores;
			wait.push_back(std::make_pair(&sync->swapchain_image_ready_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));
			signal.push_back(&sync->rendering_finished_semaphore);

			// Submit
			ste_device_queue::submit_current_queue(index,
													  wait,
													  signal);

			// Present
			if (acquired_presentation_image.next_image.image != nullptr) {
				this->presentation_surface->present(acquired_presentation_image.next_image.image_index,
													ste_device_queue::thread_queue(),
													sync->rendering_finished_semaphore);
				acquired_presentation_image = {};
			}
		});

		if (presentation_surface->test_and_clear_recreate_flag()) {
			// Recreate swap-chain and queues
			recreate_swap_chain();
		}
	}

	/**
	*	@brief	Selects a device queue and returns a pointer to it
	*
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param queue_selector		The device queue selector used to select the device queue
	*/
	template <typename selector_policy = ste_queue_selector_default_policy>
	const queue_t& select_queue(const ste_queue_selector<selector_policy> &queue_selector) const {
		auto idx = queue_selector_cache(queue_selector, queue_descriptors);
		return device_queues[idx];
	}

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_surface() const { return *presentation_surface; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_command_buffers_count() const { return command_buffers_count; }

	auto& logical_device() const { return device; }
};

}
}
