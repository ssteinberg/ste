//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_device_queue_selector_cache.hpp>
#include <ste_queue_selector.hpp>

#include <ste_device_exceptions.hpp>
#include <ste_gl_context_creation_parameters.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_device_queue.hpp>

#include <fence.hpp>
#include <vk_event.hpp>
#include <vk_semaphore.hpp>
#include <ste_resource_pool.hpp>

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

		std::weak_ptr<ste_resource_pool<fence<void>>::resource_t> fence_weak;

		presentation_sync_primitives_t() = delete;
		presentation_sync_primitives_t(const vk_logical_device &device)
			: swapchain_image_ready_semaphore(device), rendering_finished_semaphore(device)
		{}
		presentation_sync_primitives_t(presentation_sync_primitives_t &&) = default;
		presentation_sync_primitives_t &operator=(presentation_sync_primitives_t &&) = default;
	};
	struct presentation_next_image_t {
		ste_presentation_surface::acquire_next_image_return_t next_image;
		presentation_sync_primitives_t *sync{ nullptr };
	};

private:
	const ste_gl_device_creation_parameters parameters;
	const ste_queue_descriptors queue_descriptors;
	const vk_logical_device device;
	const std::unique_ptr<ste_presentation_surface> presentation_surface{ nullptr };
	std::vector<presentation_sync_primitives_t> presentation_sync_primitives;
	
	// Synchronization primitive pools
	mutable ste_resource_pool<fence<void>> fence_pool;
	mutable ste_resource_pool<vk_event> event_pool;
	mutable ste_resource_pool<vk_semaphore> semaphore_pool;

	ste_device_queue_selector_cache queue_selector_cache;
	std::vector<queue_t> device_queues;
	
	queues_and_surface_recreate_signal_type queues_and_surface_recreate_signal;

	std::uint32_t tick_count{ 0 };

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
			throw ste_device_creation_exception("queue_descriptors is empty");
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
																idx,
																&fence_pool));
		}

		device_queues = std::move(q);

		// And reset the queue selector cache
		queue_selector_cache = ste_device_queue_selector_cache();
	}

	void create_presentation_sync_primitives() {
		auto images_count = presentation_surface->get_swap_chain_images().size();

		// Create synchronization primitives
		std::vector<presentation_sync_primitives_t> v;
		v.reserve(images_count);
		for (int i = 0; i<images_count; ++i)
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
		create_presentation_sync_primitives();

		// And queues
		create_queues();

		// Signal
		queues_and_surface_recreate_signal.emit(this);
	}

public:
	/**
	*	@brief	Creates the device with presentation surface and capabilities
	*
	*	@throws ste_device_creation_exception	If creation parameters are erroneous or incompatible or creation failed for any reason
	*	@throws vk_exception	On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param gl_ctx				Context
	*	@param presentation_window	Presentation window used for rendering
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
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
																		gl_ctx.instance())),
		fence_pool(device),
		event_pool(device),
		semaphore_pool(device)
	{
		create_presentation_sync_primitives();

		// Create queues
		create_queues();
	}
	/**
	*	@brief	Creates the device without presentation capabilities ("compute-only" device)
	*
	*	@throws ste_device_creation_exception	If creation parameters are erroneous or incompatible or creation failed for any reason
	*	@throws vk_exception		On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param gl_ctx				Context
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
			   const ste_gl_context &gl_ctx)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		fence_pool(device),
		event_pool(device),
		semaphore_pool(device)
	{
		// Create queues
		create_queues();
	}
	~ste_device() noexcept {}

	ste_device(ste_device &&) = default;
	ste_device &operator=(ste_device &&) = default;

	/**
	*	@brief	Performs schedules work, cleans up resources, etc.
	*			Might stall if swap-chain recreation is required.
	*			
	*	@throws ste_device_exception	On internal error during swap-chain recreation
	*	@throws vk_exception			On Vulkan error during swap-chain recreation
	*/
	void tick() {
		if (presentation_surface->test_and_clear_recreate_flag()) {
			// Recreate swap-chain and queues
			recreate_swap_chain();
		}

		auto index = tick_count++ % get_swap_chain_images_count();
		auto& sync = presentation_sync_primitives[index];
		if (auto fence_ptr = sync.fence_weak.lock())
			(**fence_ptr).wait_idle();

		// Tick queues
		for (auto &q : device_queues)
			q->tick();
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

		// Choose index of next sync primitives
		auto index = tick_count % get_swap_chain_images_count();

		queue->enqueue([this, index]() {
			auto* sync = &presentation_sync_primitives[index];

			assert(acquired_presentation_image.sync == nullptr && "Last acquired image was not presented!");

			// Acquire next presentation image
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
	*	@brief	Submits a presentation batch and presents the acquired presentation image.
	*			Must be called from an enqueued task.
	*
	*			Only available for presentation-capable devices.
	*
	*	@throws	ste_device_not_queue_thread_exception	If thread not a queue thread
	*
	*	@param presentation_batch	Command batch which does the rendering to the acquired image
	*	@param wait_semaphores		See vk_queue::submit
	*	@param signal_semaphores	See vk_queue::submit
	*/
	void submit_and_present(ste_device_queue_batch &&presentation_batch,
							const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
							const std::vector<const vk_semaphore*> &signal_semaphores) {
		if (!ste_device_queue::is_queue_thread()) {
			throw ste_device_not_queue_thread_exception();
		}

		auto* sync = acquired_presentation_image.sync;
		assert(acquired_presentation_image.sync != nullptr && "Present called without acquire!");

		// Use the presentation batch's fence for host synchronization
		sync->fence_weak = presentation_batch.get_fence();

		// Synchronize submission with presentation
		auto wait = wait_semaphores;
		auto signal = signal_semaphores;
		wait.push_back(std::make_pair(&sync->swapchain_image_ready_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));
		signal.push_back(&sync->rendering_finished_semaphore);

		// Submit
		ste_device_queue::submit_batch(std::move(presentation_batch),
									   wait,
									   signal);

		// Present
		if (acquired_presentation_image.next_image.image != nullptr) {
			this->presentation_surface->present(acquired_presentation_image.next_image.image_index,
												ste_device_queue::thread_queue(),
												sync->rendering_finished_semaphore);
			acquired_presentation_image = {};
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

	/**
	*	@brief	Waits idly for all the queues and the device to finish processing
	*/
	void wait_idle() const {
		for (auto &q : device_queues) {
			q->wait_idle();
		}
		device.wait_idle();
	}

	/**
	*	@brief	Thread-safe fence pool
	*/
	auto& get_fence_pool() const { return fence_pool; }
	/**
	*	@brief	Thread-safe event pool
	*/
	auto& get_event_pool() const { return event_pool; }
	/**
	*	@brief	Thread-safe semaphore pool
	*/
	auto& get_semaphore_pool() const { return semaphore_pool; }

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_surface() const { return *presentation_surface; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_swap_chain_images_count() const { return presentation_surface->get_swap_chain_images().size(); }

	auto& logical_device() const { return device; }
};

}
}
