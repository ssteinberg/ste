//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device.hpp>
#include <ste_device_exceptions.hpp>
#include <pipeline_stage.hpp>

#include <presentation_engine_sync_semaphores.hpp>
#include <device_queue_presentation_batch.hpp>

#include <ste_device_queue.hpp>
#include <ste_queue_selector.hpp>
#include <wait_semaphore.hpp>

#include <signal.hpp>

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <lib/unique_ptr.hpp>
#include <lib/aligned_padded_ptr.hpp>
#include <optional.hpp>
#include <chrono>

namespace ste {
namespace gl {

class presentation_engine {
private:
	using batch_fence_ptr_t = device_queue_presentation_batch<>::fence_ptr_strong_t;

	struct image_presentation_sync_t {
		batch_fence_ptr_t fence_ptr;
		presentation_engine_sync_semaphores semaphores;
	};

	struct shared_data_t {
		std::atomic<std::uint32_t> acquired_images{ 0 };
		std::mutex acquire_mutex;
		std::condition_variable acquire_cv;

		std::atomic<std::uint64_t> last_acquire_time_ns;
		std::atomic<std::uint64_t> frame_time_ns;
	};

private:
	const ste_device *device;
	std::uint32_t max_frame_lag;

	// Data shared between presentation threads
	lib::aligned_padded_ptr<shared_data_t> shared_data;
	// Presentation synchronization primitives
	lib::vector<image_presentation_sync_t> presentation_sync_primitives;

	ste_device::queues_and_surface_recreate_signal_type::connection_type surface_recreate_signal_connection;

private:
	void create_presentation_fences_storage() {
		auto count = device->get_swap_chain_images_count();

		lib::vector<image_presentation_sync_t> v;
		v.reserve(count);
		for (std::uint32_t i = 0; i < count; ++i) {
			image_presentation_sync_t sync_object = {
				nullptr,
				{
					device->get_sync_primitives_pools().semaphores().claim(),
					device->get_sync_primitives_pools().semaphores().claim()
				}
			};
			v.push_back(std::move(sync_object));
		}

		this->presentation_sync_primitives = std::move(v);
	}

	void tick() {
		auto now = std::chrono::high_resolution_clock::now();
		auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
		auto prev_ns = shared_data->last_acquire_time_ns.load(std::memory_order_acquire);

		shared_data->frame_time_ns.store(ns - prev_ns, std::memory_order_release);
		shared_data->last_acquire_time_ns.store(ns, std::memory_order_release);
	}

	template <typename selector_policy = ste_queue_selector_default_policy>
	auto internal_acquire_next_image(const ste_queue_selector<selector_policy> &queue_selector) {
		if (ste_device_queue::is_queue_thread()) {
			throw ste_device_exception("Should be called from a main thread");
		}

		// Acquire a couple of semaphores and a fence
		auto semaphores = presentation_engine_sync_semaphores(device->get_sync_primitives_pools().semaphores().claim(),
															  device->get_sync_primitives_pools().semaphores().claim());
		auto fence = lib::allocate_shared<batch_fence_ptr_t::element_type>(device->get_sync_primitives_pools().shared_fences().claim());

		// Wait for outstanding presentations
		auto max_outstanding = std::min<std::uint32_t>(max_frame_lag,
													   device->get_surface().get_max_allowed_acquired_swap_chain_images());
		if (shared_data->acquired_images.load(std::memory_order_acquire) >= max_outstanding) {
			// Must present before we can acquire another image. Wait.
			std::unique_lock<std::mutex> lk(shared_data->acquire_mutex);
			shared_data->acquire_cv.wait(lk, [this, max_outstanding] {
				return shared_data->acquired_images.load(std::memory_order_acquire) < max_outstanding;
			});
		}

		// Update presentation time counters
		tick();

		// Acquire next presentation image
		auto next_image_descriptor =
			device->get_surface().acquire_next_swapchain_image(*semaphores.swapchain_image_ready_semaphore);
		shared_data->acquired_images.fetch_add(1, std::memory_order_relaxed);

		// Wait for previous presentation to this image index to end
		auto& sync = presentation_sync_primitives[next_image_descriptor.image_index];
		if (sync.fence_ptr != nullptr)
			(*sync.fence_ptr)->get_wait();

		// Hold onto new presentation semaphores, and fence. Releasing old.
		sync.semaphores = std::move(semaphores);
		sync.fence_ptr = std::move(fence);

		return std::make_pair(std::reference_wrapper<image_presentation_sync_t>(sync), next_image_descriptor);
	}

	template <typename BatchUserData>
	void internal_submit_and_present(lib::unique_ptr<device_queue_presentation_batch<BatchUserData>> &&presentation_batch,
									 const lib::vector<wait_semaphore> &wait_semaphores,
									 const lib::vector<const semaphore*> &signal_semaphores) {
		if (!ste_device_queue::is_queue_thread()) {
			throw ste_device_not_queue_thread_exception();
		}

		auto acquired_presentation_image = presentation_batch->image_to_present;
		auto presentation_semaphores = presentation_batch->semaphores;

		// Synchronize submission with presentation
		auto wait = wait_semaphores;
		auto signal = signal_semaphores;

		const semaphore &swapchain_image_ready_semaphore = presentation_semaphores->swapchain_image_ready_semaphore;
		const semaphore &rendering_finished_semaphore = presentation_semaphores->rendering_finished_semaphore;

		wait.push_back(wait_semaphore(&swapchain_image_ready_semaphore,
									  pipeline_stage::color_attachment_output));
		signal.push_back(&rendering_finished_semaphore);

		// Submit
		ste_device_queue::submit_batch<BatchUserData>(std::move(presentation_batch),
													  wait,
													  signal);

		// Present
		if (acquired_presentation_image.image != nullptr) {
			device->get_surface().present(acquired_presentation_image.image_index,
										  ste_device_queue::thread_queue(),
										  presentation_semaphores->rendering_finished_semaphore);

			// Notify any waiting for present
			shared_data->acquired_images.fetch_add(-1, std::memory_order_release);
			shared_data->acquire_cv.notify_one();
		}
	}

public:
	/**
	*	@brief	Creates the presentation engine
	*
	*	@param device			Parent device
	*	@param max_frame_lag		Maximal allowed count of outstanding presentations. Intrinsically limited to the amount of swap
	*							chain images. Must be positive.
	*/
	presentation_engine(const ste_device &device,
						optional<std::uint32_t> max_frame_lag = none)
		: device(&device)
	{
		// If max_frame_lag parameter is provided by consumer, use it.
		// Otherwise use the user selected simultaneous_presentation_frames value, if exists.
		// Otherwise unlimited.
		if (!max_frame_lag)
			max_frame_lag = device.get_creation_parameters().simultaneous_presentation_frames;
		this->max_frame_lag = max_frame_lag ?
			max_frame_lag.get() :
			std::numeric_limits<std::uint32_t>::max();
		assert(this->max_frame_lag > 0);

		// Create per swap-chain image fence, used for presentation sync.
		this->create_presentation_fences_storage();

		// Connect signal to get notifications of presentation surface rebuild
		surface_recreate_signal_connection = make_connection(device.get_queues_and_surface_recreate_signal(), [this](const ste_device*) {
			// Recreate fences
			this->presentation_sync_primitives.clear();
			this->create_presentation_fences_storage();
		});
	}
	~presentation_engine() noexcept {}

	presentation_engine(presentation_engine&&) = default;
	presentation_engine &operator=(presentation_engine&&) = default;

	/**
	*	@brief	Acquires the next presentation image information and allocates a command batch that will be used for presentation.
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
		auto next_image = internal_acquire_next_image(queue_selector);
		auto& queue = device->select_queue(queue_selector);

		// Allocate batch and pass it next presentation image information
		using batch_t = device_queue_presentation_batch<UserData>;
		auto batch = queue.template allocate_batch_custom<batch_t>(batch_t::batch_ctor(),
																	next_image.second,
																	&next_image.first.semaphores,
																	next_image.first.fence_ptr,
																	std::forward<UserDataArgs>(user_data_args)...);

		return batch;
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
	void submit_and_present(lib::unique_ptr<device_queue_presentation_batch<BatchUserData>> &&presentation_batch,
							const lib::vector<wait_semaphore> &wait_semaphores = {},
							const lib::vector<const semaphore*> &signal_semaphores = {}) {
		internal_submit_and_present(std::move(presentation_batch),
									wait_semaphores,
									signal_semaphores);
	}

	/**
	 *	@brief	Returns the last measured frame time, in nanoseconds.
	 */
	auto get_frame_time() const {
		return shared_data->frame_time_ns.load(std::memory_order_acquire);
	}
};

}
}
