//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_engine.hpp>

#include <ste_window.hpp>
#include <vk_physical_device_descriptor.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_device_queue_selector_cache.hpp>
#include <ste_device_sync_primitives_pools.hpp>
#include <ste_queue_selector.hpp>
#include <ste_device_pipeline_cache.hpp>
#include <pipeline_binding_set_pool.hpp>

#include <ste_gl_context_creation_parameters.hpp>
#include <ste_presentation_surface.hpp>
#include <ste_gl_context.hpp>
#include <ste_device_queue.hpp>

#include <signal.hpp>

#include <memory>
#include <vector>
#include <aligned_ptr.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class ste_device : public allow_type_decay<ste_device, vk::vk_logical_device> {
public:
	using queues_and_surface_recreate_signal_type = signal<const ste_device*>;

private:
	using queue_t = std::unique_ptr<ste_device_queue>;
	using queues_t = std::vector<queue_t>;

private:
	const ste_gl_device_creation_parameters parameters;
	const ste_queue_descriptors queue_descriptors;
	const vk::vk_logical_device device;

	// Pipeline cache
	ste_device_pipeline_cache shared_pipeline_cache;
	// And binding sets pool
	mutable pipeline_binding_set_pool device_binding_set_pool;

	// Synchronization primitive pools
	aligned_ptr<ste_device_sync_primitives_pools> sync_primitives_pools;

	// Queues
	const queues_t device_queues;

	// Presentation surface
	const std::unique_ptr<ste_presentation_surface> presentation_surface{ nullptr };

	mutable queues_and_surface_recreate_signal_type queues_and_surface_recreate_signal;
	ste_device_queue_selector_cache queue_selector_cache;

private:
	static vk::vk_logical_device create_vk_virtual_device(const vk::vk_physical_device_descriptor &physical_device,
														  const VkPhysicalDeviceFeatures &requested_features,
														  const ste_queue_descriptors &queue_descriptors,
														  std::vector<const char*> device_extensions = {});
	static queues_t create_queues(const vk::vk_logical_device &device,
								  const ste_queue_descriptors &queue_descriptors,
								  ste_device_sync_primitives_pools *sync_primitives_pools);

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
	*	@param engine				StE engine object
	*	@param gl_ctx				Context
	*	@param presentation_window	Presentation window used for rendering
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
			   ste_engine &engine,
			   const ste_gl_context &gl_ctx,
			   const ste_window &presentation_window)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		shared_pipeline_cache(device,
							  &engine.cache(),
							  this->name()),
		device_binding_set_pool(device),
		sync_primitives_pools(device),
		device_queues(create_queues(device,
									queue_descriptors,
									&*sync_primitives_pools)),
		presentation_surface(std::make_unique<ste_presentation_surface>(parameters,
																		&device,
																		presentation_window,
																		gl_ctx.instance()))
	{}
	/**
	*	@brief	Creates the device without presentation capabilities ("compute-only" device)
	*
	*	@throws ste_device_creation_exception	If creation parameters are erroneous or incompatible or creation failed for any reason
	*	@throws vk_exception		On Vulkan error
	*
	*	@param parameters			Device creation parameters
	*	@param queue_descriptors	Queues descriptors. Influences amount and families of created device queues.
	*	@param engine				StE engine object
	*	@param gl_ctx				Context
	*/
	ste_device(const ste_gl_device_creation_parameters &parameters,
			   const ste_queue_descriptors &queue_descriptors,
			   ste_engine &engine,
			   const ste_gl_context &gl_ctx)
		: parameters(parameters),
		queue_descriptors(queue_descriptors),
		device(create_vk_virtual_device(parameters.physical_device,
										parameters.requested_device_features,
										queue_descriptors,
										parameters.additional_device_extensions)),
		shared_pipeline_cache(device,
							  &engine.cache(),
							  this->name()),
		device_binding_set_pool(device),
		sync_primitives_pools(device),
		device_queues(create_queues(device,
									queue_descriptors,
									&*sync_primitives_pools))
	{}
	~ste_device() noexcept {}

	ste_device(ste_device &&) = default;
	ste_device &operator=(ste_device &&) = delete;

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

	/**
	*	@brief	Thread-safe pipeline cache generator
	*/
	auto& pipeline_cache() const { return shared_pipeline_cache; }

	/**
	 *	@brief	Thread-safe pool of pipeline binding sets
	 */
	auto& binding_set_pool() const { return device_binding_set_pool; }

	/**
	*	@brief	Human readable device name
	*/
	std::string name() const { return std::string(parameters.physical_device.properties.deviceName); }

	auto& get_queues_and_surface_recreate_signal() const { return queues_and_surface_recreate_signal; }

	auto& get_creation_parameters() const { return parameters; }
	auto& get_surface() const { return *presentation_surface; }
	auto& get_queue_descriptors() const { return queue_descriptors; }

	std::uint32_t get_swap_chain_images_count() const { return presentation_surface->get_swap_chain_images().size(); }

	/**
	*	@brief	Get device handle
	*/
	auto& get() const { return device; }
};

}
}
