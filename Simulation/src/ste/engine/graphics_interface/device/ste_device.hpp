//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_device_queue_selector_cache.hpp>
#include <ste_device_sync_primitives_pools.hpp>
#include <ste_device_presentation_sync_semaphores.hpp>
#include <ste_queue_selector.hpp>

#include <ste_device_exceptions.hpp>
#include <ste_gl_context_creation_parameters.hpp>
#include <ste_device_queue_presentation_batch.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_device_queue.hpp>

#include <signal.hpp>

#include <memory>
#include <vector>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <aligned_ptr.hpp>

namespace StE {
namespace GL {

class ste_device {
public:
	using queues_and_surface_recreate_signal_type = signal<const ste_device*>;

private:
	using queue_t = std::unique_ptr<ste_device_queue>;
	using queues_t = std::vector<queue_t>;
	using batch_fence_ptr_t = ste_device_queue_presentation_batch<>::fence_ptr_strong_t;

	struct image_presentation_sync_t {
		batch_fence_ptr_t fence_ptr;
		ste_device_presentation_sync_semaphores semaphores;
	};

	struct shared_data_t {
		std::atomic<std::uint32_t> acquired_images{ 0 };
		std::mutex acquire_mutex;
		std::condition_variable acquire_cv;
	};

private:
	const ste_gl_device_creation_parameters parameters;
	const ste_queue_descriptors queue_descriptors;
	const vk_logical_device device;

	// Synchronization primitive pools
	aligned_ptr<ste_device_sync_primitives_pools> sync_primitives_pools;
	// Data shared between threads
	aligned_ptr<shared_data_t> shared_data;

	// Queues
	const queues_t device_queues;

	// Presentation surface
	const std::unique_ptr<ste_presentation_surface> presentation_surface{ nullptr };
	// And synchronization primitives
	std::vector<image_presentation_sync_t> presentation_sync_primitives;

	queues_and_surface_recreate_signal_type queues_and_surface_recreate_signal;
	ste_device_queue_selector_cache queue_selector_cache;

private:
	static vk_logical_device create_vk_virtual_device(const GL::vk_physical_device_descriptor &physical_device,
													  const VkPhysicalDeviceFeatures &requested_features,
													  const ste_queue_descriptors &queue_descriptors,
													  std::vector<const char*> device_extensions = {});
	static queues_t create_queues(const vk_logical_device &device, 
								  const ste_queue_descriptors &queue_descriptors,
								  ste_device_sync_primitives_pools *sync_primitives_pools);
	
	void create_presentation_fences_storage();
	void recreate_swap_chain();

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
		sync_primitives_pools(device),
		device_queues(create_queues(device,
									queue_descriptors,
									&*sync_primitives_pools)),
		presentation_surface(std::make_unique<ste_presentation_surface>(parameters,
																		&device,
																		presentation_window,
																		gl_ctx.instance()))
	{
		create_presentation_fences_storage();
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
		sync_primitives_pools(device),
		device_queues(create_queues(device,
									queue_descriptors,
									&*sync_primitives_pools))
	{}
	~ste_device() noexcept {}

	ste_device(ste_device &&) = default;
	ste_device &operator=(ste_device &&) = default;

	/**
	*	@brief	Performs schedules work, cleans up resources, etc.
	*			Might stall if swap-chain recreation is required.
	*			
	*	@throws ste_device_exception	On internal error during swap-chain recreation
	*	@throws vk_exception			On Vulkan error during swap-chain recreation
	*	@throws ste_engine_glfw_exception	On windowing system error
	*/
	void tick() {
		if (presentation_surface->test_and_clear_recreate_flag()) {
			// Recreate swap-chain and queues
			recreate_swap_chain();
		}

		// Tick queues
		for (auto &q : device_queues)
			q->tick();
	}

	/**
	*	@brief	Acquires the next presentation image information and allocates a coomand batch that will be used for presentation.
	*			Must be called from main thread.
	*			The command batch shall be consumed with a call to submit_and_present().
	*
	*			Only available for presentation-capable devices.
	*
	*	@throws ste_engine_exception	If no compatible queue can be found
	*	@throws ste_device_exception	If called form a queue thread
	*	@throws vk_exception			On Vulkan error
	*
	*	@param	queue_selector		The device queue selector used to select the device queue to use
	*	@param	user_data_args		Arguments used to initialze the user data structure in the allocated batch
	*/
	template <typename UserData = void, typename selector_policy = ste_queue_selector_default_policy, typename... UserDataArgs>
	auto allocate_presentation_command_batch(const ste_queue_selector<selector_policy> &queue_selector,
											 UserDataArgs&&... user_data_args) {
		if (ste_device_queue::is_queue_thread()) {
			throw ste_device_exception("Should be called from a main thread");
		}

		auto& queue = select_queue(queue_selector);

		// Acquire a couple of semaphores
		auto semaphores = ste_device_presentation_sync_semaphores(sync_primitives_pools->semaphores().claim(),
																  sync_primitives_pools->semaphores().claim());
		auto fence = std::make_shared<batch_fence_ptr_t::element_type>(sync_primitives_pools->shared_fences().claim());

		// Wait for images to become available
		if (shared_data->acquired_images.load(std::memory_order_acquire) >= get_swap_chain_images_count() - 1) {
			// Must present before we can acquire another image. Wait.
			std::unique_lock<std::mutex> lk(shared_data->acquire_mutex);
			shared_data->acquire_cv.wait(lk, [this]{
				return shared_data->acquired_images.load(std::memory_order_acquire) < get_swap_chain_images_count() - 1;
			});
		}

		// Acquire next presentation image
		auto next_image_descriptor =
			presentation_surface->acquire_next_swapchain_image(*semaphores.swapchain_image_ready_semaphore);
		shared_data->acquired_images.fetch_add(1, std::memory_order_relaxed);

		// Wait for previous presentation to this image index to end
		auto& sync = presentation_sync_primitives[next_image_descriptor.image_index];
		if (sync.fence_ptr != nullptr)
			(*sync.fence_ptr)->get();

		// Hold onto new presentation semaphores, and fence. Releasing old.
		sync.semaphores = std::move(semaphores);
		sync.fence_ptr = std::move(fence);

		// Allocate batch
		auto batch = queue->template allocate_batch_custom<ste_device_queue_presentation_batch<UserData>>(sync.fence_ptr,
																										  std::forward<UserDataArgs>(user_data_args)...);

		// And save next presentation image information
		batch->image_to_present = next_image_descriptor;
		batch->semaphores = &sync.semaphores;

		return batch;
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
	*	@brief	Enqueues a batch submission and presention of the acquired presentation image tasks.
	*			Must be called from a queue thread.
	*
	*			Only available for presentation-capable devices.
	*
	*	@throws ste_engine_exception	If no compatible queue can be found
	*	@throws ste_device_not_queue_thread_exception	If not called form a queue thread
	*
	*	@param presentation_batch	Command batch which does the rendering to the acquired image
	*	@param wait_semaphores		See vk_queue::submit
	*	@param signal_semaphores	See vk_queue::submit
	*/
	template <typename BatchUserData>
	void submit_and_present(std::unique_ptr<ste_device_queue_presentation_batch<BatchUserData>> &&presentation_batch,
							const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
							const std::vector<const vk_semaphore*> &signal_semaphores) {
		if (!ste_device_queue::is_queue_thread()) {
			throw ste_device_not_queue_thread_exception();
		}

		auto acquired_presentation_image = presentation_batch->image_to_present;
		auto presentation_semaphores = presentation_batch->semaphores;

		// Synchronize submission with presentation
		auto wait = wait_semaphores;
		auto signal = signal_semaphores;
		wait.push_back(std::make_pair(&*presentation_semaphores->swapchain_image_ready_semaphore,
									  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));
		signal.push_back(&*presentation_semaphores->rendering_finished_semaphore);

		// Submit
		ste_device_queue::submit_batch(std::move(presentation_batch),
									   wait,
									   signal);

		// Present
		if (acquired_presentation_image.image != nullptr) {
			this->presentation_surface->present(acquired_presentation_image.image_index,
												ste_device_queue::thread_queue(),
												*presentation_semaphores->rendering_finished_semaphore);

			// Notify any waiting for present
			shared_data->acquired_images.fetch_add(-1, std::memory_order_release);
			shared_data->acquire_cv.notify_one();
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
	*	@brief	Selects a device queue and returns a pointer to it
	*
	*	@throws ste_engine_exception	If no compatible queue can be found
	*
	*	@param index			Queue index to select
	*/
	const queue_t& select_queue(const ste_device_queue::queue_index_t &index) const {
		if (index < device_queues.size())
			return device_queues[index];
		
		throw ste_engine_exception("Queue with the desired index not found");
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
	*	@brief	Thread-safe pools for device synchronization primitives
	*/
	auto& get_sync_primitives_pools() const { return *sync_primitives_pools; }

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_surface() const { return *presentation_surface; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_swap_chain_images_count() const { return presentation_surface->get_swap_chain_images().size(); }

	auto& logical_device() const { return device; }
};

}
}
