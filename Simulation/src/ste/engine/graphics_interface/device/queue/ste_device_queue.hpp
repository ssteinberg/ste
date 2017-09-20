//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queues_protocol.hpp>
#include <ste_device_queue_batch.hpp>
#include <ste_device_exceptions.hpp>

#include <pipeline_stage.hpp>

#include <vk_queue.hpp>
#include <vk_logical_device.hpp>
#include <ste_device_queue_command_pool.hpp>
#include <ste_device_queue_secondary_buffer_allocator.hpp>
#include <vk_command_buffers.hpp>
#include <ste_resource_pool.hpp>
#include <ste_device_sync_primitives_pools.hpp>
#include <wait_semaphore.hpp>

#include <condition_variable>
#include <mutex>
#include <future>
#include <utility>
#include <lib/unique_ptr.hpp>
#include <lib/vector.hpp>
#include <lib/list.hpp>
#include <atomic>
#include <lib/aligned_padded_ptr.hpp>

#include <lib/concurrent_queue.hpp>
#include <interruptible_thread.hpp>
#include <thread_pool_task.hpp>

#include <function_traits.hpp>
#include <type_traits>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class ste_device_queue : public allow_type_decay<ste_device_queue, vk::vk_queue<>> {
public:
	using task_t = unique_thread_pool_type_erased_task<>;
	template <typename R>
	using enqueue_task_t = unique_thread_pool_task<R>;
	using queue_index_t = std::uint32_t;

	using secondary_buffer_allocator_t = ste_device_queue_secondary_buffer_allocator<ste_resource_pool<ste_device_queue_command_pool>::resource_t>;
	using secondary_buffer_t = secondary_buffer_allocator_t::buffer_t;

private:
	using shared_fence_t = ste_device_queue_batch<void>::fence_t;
	using task_queue_t = lib::concurrent_queue<task_t>;

	struct shared_data_t {
		mutable std::mutex m;
		mutable std::condition_variable notifier;

		task_queue_t task_queue;
	};

private:
	const queue_index_t queue_index;
	const vk::vk_queue<> queue;
	const ste_queue_descriptor descriptor;

	ste_device_sync_primitives_pools::shared_fence_pool_t *shared_fence_pool;

	lib::list<lib::unique_ptr<ste_device_queue_batch_base>> submitted_batches;

	lib::aligned_padded_ptr<shared_data_t> shared_data;
	ste_resource_pool<ste_device_queue_command_pool> pool;
	lib::unique_ptr<interruptible_thread> thread;

	secondary_buffer_allocator_t secondary_buffer_allocator;

private:
	static thread_local ste_device_queue *static_device_queue_ptr;
	static thread_local const vk::vk_queue<> *static_queue_ptr;
	static thread_local queue_index_t static_queue_index;

private:
	static auto& thread_device_queue() { return *static_device_queue_ptr; }

public:
	/**
	*	@brief	Returns the Vulkan queue handle for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto& thread_queue() { return *static_queue_ptr; }
	/**
	*	@brief	Returns the reported queue index for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto thread_queue_index() { return static_queue_index; }
	/**
	*	@brief	Checks if current thread is a queue thread
	*/
	static bool is_queue_thread() { return thread_queue_index() != 0xFFFFFFFF; }

	/**
	*	@brief	Allocates a new command batch.
	*			Must be called from an enqueued task.
	*			UserData can be any structure that holds some user data. The user data is accessible by the public user_data
	*			member in the returned batch, and is constructed with the supplied user_data_args parameter pack. The user data
	*			should be used to hold data used by the batch commands, and will be deallocated with the batch only once
	*			processing is complete by the device.
	*/
	template <typename UserData = void, typename... UserDataArgs>
	static auto thread_allocate_batch(UserDataArgs&&... user_data_args) {
		using batch_t = ste_device_queue_batch<UserData>;
		return lib::allocate_unique<batch_t>(batch_t::ctor{},
											 thread_queue_index(),
											 thread_device_queue().pool.claim(),
											 lib::allocate_shared<shared_fence_t>(thread_device_queue().shared_fence_pool->claim()),
											 std::forward<UserDataArgs>(user_data_args)...);
	}
	/**
	*	@brief	Allocates a new command batch.
	*			Must be called from an enqueued task.
	*/
	template <typename Batch, typename... Args>
	static auto thread_allocate_batch_custom(Args&&... custom_args) {
		return lib::allocate_unique<Batch>(Batch::ctor{},
										   thread_queue_index(),
										   lib::allocate_shared<shared_fence_t>(thread_device_queue().shared_fence_pool->claim()),
										   std::forward<Args>(custom_args)...);
	}
	/**
	*	@brief	Allocates a new command batch.
	*			Must be called from an enqueued task.
	*/
	template <typename Batch, typename... Args>
	static auto thread_allocate_batch_pool_custom(Args&&... custom_args) {
		return lib::allocate_unique<Batch>(Batch::ctor{},
										   thread_queue_index(),
										   thread_device_queue().pool.claim(),
										   lib::allocate_shared<shared_fence_t>(thread_device_queue().shared_fence_pool->claim()),
										   std::forward<Args>(custom_args)...);
	}

	/**
	*	@brief	Allocates a secondary command buffer owned by this queue.
	*			A secondary command buffer can executed by a primary buffer beloning to a batch allocated from this queue or a queue with identical family index.
	*/
	static auto thread_allocate_secondary_command_buffer() {
		return thread_device_queue().secondary_buffer_allocator.allocate_secondary_buffer();
	}

	/**
	*	@brief	Consumes a command batch to the queue, submitting its command buffers.
	*			Must be called from an enqueued task.
	*
	*	@throws	ste_device_not_queue_thread_exception	If thread not a queue thread
	*	@throws	ste_device_exception	If batch was not created on this queue, batch's fence will be set to a ste_device_exception.
	*
	*	@param	batch			Command batch to submit
	*/
	static void submit_batch(lib::unique_ptr<ste_device_queue_batch_base> &&batch) {
		if (!is_queue_thread()) {
			throw ste_device_not_queue_thread_exception();
		}

		try {
			if (batch->queue_index == thread_queue_index()) {
				// Submit
				batch->submit(thread_queue());
				// And set submitted flag
				batch->submitted = true;

				// Hold onto the batch, release resources only once the device is done with it
				thread_device_queue().submitted_batches.emplace_back(std::move(batch));
			}
			else {
				throw ste_device_exception("Batch created on a different queue");
			}
		}
		catch (...) {
			(*batch->get_fence_ptr())->set_exception(std::current_exception());
		}
	}

private:
	void create_worker();
	void prune_submitted_batches();

	bool is_thread_this_queue_thread() const { return thread_queue_index() == queue_index; }

public:
	ste_device_queue(const vk::vk_logical_device<> &device,
					 std::uint32_t device_family_index,
					 ste_queue_descriptor descriptor,
					 queue_index_t queue_index,
					 ste_device_sync_primitives_pools::shared_fence_pool_t *shared_fence_pool,
					 const char *name)
		: queue_index(queue_index),
		queue(device, 
			  descriptor.family, 
			  device_family_index,
			  name),
		descriptor(descriptor),
		shared_fence_pool(shared_fence_pool),
		pool(device, descriptor),
		secondary_buffer_allocator(pool.claim())
	{
		// Create the queue worker thread
		create_worker();
	}
	~ste_device_queue() noexcept {
		thread->interrupt();

		do { shared_data->notifier.notify_all(); } while (!shared_data->m.try_lock());
		shared_data->m.unlock();

		thread->join();
		prune_submitted_batches();
		queue.wait_idle();
	}

	ste_device_queue(ste_device_queue &&q) = default;
	ste_device_queue &operator=(ste_device_queue &&) = delete;
	ste_device_queue(const ste_device_queue &) = delete;
	ste_device_queue &operator=(const ste_device_queue &) = delete;

	/**
	*	@brief	Releases some pools' resources.
	*/
	void tick() {
		enqueue([=]() {
			thread_device_queue().prune_submitted_batches();
		});
	}

	/**
	*	@brief	Allocates a new command batch.
	*			UserData can be any structure that holds some user data. The user data is accessible by the public user_data
	*			member in the returned batch, and is constructed with the supplied user_data_args parameter pack. The user data
	*			should be used to hold data used by the batch commands, and will be deallocated with the batch only once
	*			processing is complete by the device.
	*/
	template <typename UserData = void, typename... UserDataArgs>
	auto allocate_batch(UserDataArgs&&... user_data_args) {
		using batch_t = ste_device_queue_batch<UserData>;
		return lib::allocate_unique<batch_t>(batch_t::ctor{},
											 queue_index,
											 pool.claim(),
											 lib::allocate_shared<shared_fence_t>(shared_fence_pool->claim()),
											 std::forward<UserDataArgs>(user_data_args)...);
	}
	/**
	*	@brief	Allocates a new command batch.
	*/
	template <typename Batch, typename... Args>
	auto allocate_batch_custom(Args&&... custom_args) {
		return lib::allocate_unique<Batch>(Batch::ctor{},
										   queue_index,
										   lib::allocate_shared<shared_fence_t>(shared_fence_pool->claim()),
										   std::forward<Args>(custom_args)...);
	}
	/**
	*	@brief	Allocates a new command batch with a command pool.
	*/
	template <typename Batch, typename... Args>
	auto allocate_batch_pool_custom(Args&&... custom_args) {
		return lib::allocate_unique<Batch>(Batch::ctor{},
										   queue_index,
										   pool.claim(),
										   lib::allocate_shared<shared_fence_t>(shared_fence_pool->claim()),
										   std::forward<Args>(custom_args)...);
	}

	/**
	 *	@brief	Allocates a secondary command buffer owned by this queue.
	 *			A secondary command buffer can executed by a primary buffer beloning to a batch allocated from this queue or a queue with identical family index.
	 */
	auto allocate_secondary_command_buffer() {
		return secondary_buffer_allocator.allocate_secondary_buffer();
	}

	/**
	*	@brief	Enqueues a task on the queue's thread
	*
	*	@param	task	Task to enqueue
	*/
	template <typename L>
	std::future<typename function_traits<L>::result_t> enqueue(L &&task) {
		using R = typename function_traits<L>::result_t;

		static_assert(function_traits<L>::arity == 0,
					  "task must take no arguments");

		// Enqueue
		enqueue_task_t<R> f(std::forward<L>(task));
		auto future = f.get_future();

		shared_data->task_queue.push(std::move(f));
		shared_data->notifier.notify_one();

		return future;
	}

	/**
	*	@brief	Waits idly for the queue to finish processing
	*
	*	@throws	ste_device_exception	If thread is a queue thread
	*/
	void wait_idle() const {
		if (is_thread_this_queue_thread()) {
			throw ste_device_exception("Deadlock");
		}

		std::atomic_thread_fence(std::memory_order_acquire);
		while (!shared_data->task_queue.is_empty_hint()) {
			shared_data->notifier.notify_all();
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}

		// And wait idle (for weired implementations that would signal the fence before the queue is complete)
		queue.wait_idle();
	}

	/**
	*	@brief	Waits idly for the queue to finish processing and locks the queue. Until the queue is unlocked, no new enqueued tasks will be processed.
	*
	*	@throws	ste_device_exception	If thread is a queue thread
	 */
	void wait_lock() const {
		if (is_thread_this_queue_thread()) {
			throw ste_device_exception("Deadlock");
		}

		do { shared_data->notifier.notify_all(); } while (!shared_data->m.try_lock());
		queue.wait_idle();
	}

	/**
	 *	@brief	Unlocks the queue after locking it, resuming queue processing.
	 */
	void unlock() const {
		shared_data->m.unlock();
	}

	auto &queue_descriptor() const { return descriptor; }
	auto index() const { return queue_index; }

	/**
	*	@brief	Get queue handle
	*/
	auto &get() const { return queue; }
};

}
}
