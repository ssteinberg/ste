//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queues_protocol.hpp>
#include <ste_device_queue_batch.hpp>
#include <ste_device_exceptions.hpp>

#include <vk_queue.hpp>
#include <vk_logical_device.hpp>
#include <ste_device_queue_command_pool.hpp>
#include <vk_command_buffers.hpp>
#include <ste_resource_pool.hpp>
#include <fence.hpp>

#include <condition_variable>
#include <mutex>
#include <future>
#include <utility>
#include <memory>
#include <vector>
#include <list>
#include <atomic>

#include <concurrent_queue.hpp>
#include <interruptible_thread.hpp>
#include <thread_pool_task.hpp>

#include <function_traits.hpp>
#include <type_traits>

namespace StE {
namespace GL {

class ste_device_queue {
public:
	using task_t = unique_thread_pool_type_erased_task<>;
	template <typename R>
	using enqueue_task_t = unique_thread_pool_task<R>;
	using fence_t = fence<void>;

private:
	std::uint32_t queue_index;
	const vk_queue queue;
	const ste_queue_descriptor descriptor;

	ste_resource_pool<fence<void>> *fence_pool;

	ste_resource_pool<ste_device_queue_command_pool> pool;
	std::list<ste_device_queue_batch> submitted_batches;

	mutable std::mutex m;
	mutable std::condition_variable notifier;
	std::unique_ptr<interruptible_thread> thread;

	concurrent_queue<task_t> task_queue;

private:
	static thread_local ste_device_queue *static_device_queue_ptr;
	static thread_local const vk_queue *static_queue_ptr;
	static thread_local std::uint32_t static_queue_index;

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
	*/
	static auto thread_allocate_batch() {
		return ste_device_queue_batch(thread_queue_index(),
									  thread_device_queue().pool.claim(),
									  thread_device_queue().fence_pool->claim());
	}

	/**
	*	@brief	Submits the command batch to the queue.
	*			Must be called from an enqueued task.
	*			
	*	@throws	ste_device_not_queue_thread_exception	If thread not a queue thread
	*	@throws	ste_device_exception	If batch was not created on this queue, batch's fence will be set to a ste_engine_exception.
	*
	*	@param	batch				Command batch to submit
	*	@param	wait_semaphores		See vk_queue::submit
	*	@param	signal_semaphores	See vk_queue::submit
	*/
	static void submit_batch(ste_device_queue_batch &&batch,
							 const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores = {},
							 const std::vector<const vk_semaphore*> &signal_semaphores = {}) {
		if (!is_queue_thread()) {
			throw ste_device_not_queue_thread_exception();
		}

		// Copy command buffers' handle for submission
		std::vector<vk_command_buffer> command_buffers;
		command_buffers.reserve(batch.command_buffers.size());
		for (auto &b : batch)
			command_buffers.push_back(static_cast<vk_command_buffer>(b));

		auto& fence = (*batch.get_fence());

		try {
			// Hold onto the batch, release resources only once the device is done with it
			thread_device_queue().submitted_batches.push_back(std::move(batch));

			if (batch.queue_index == thread_queue_index()) {
				// Submit finalized buffers
				thread_queue().submit(command_buffers,
									  wait_semaphores,
									  signal_semaphores,
									  &fence->get_fence());

				// And signal fence future
				fence->signal();
			}
			else {
				assert(false && "Batch created on a different queue");
				throw ste_device_exception("Batch created on a different queue");
			}
		}
		catch (...) {
			fence->set_exception(std::current_exception());
		}
	}

private:
	void create_worker() {
		auto idx = queue_index;
		thread = std::make_unique<interruptible_thread>([this, idx]() {
			// Set the thread_local globals to this thread's parameters
			ste_device_queue::static_device_queue_ptr = this;
			ste_device_queue::static_queue_ptr = &this->queue;
			ste_device_queue::static_queue_index = idx;

			for (;;) {
				if (interruptible_thread::is_interruption_flag_set()) return;

				std::unique_ptr<task_t> task;
				{
					std::unique_lock<std::mutex> l(this->m);
					if (interruptible_thread::is_interruption_flag_set()) return;

					this->notifier.wait(l, [&]() {
						return interruptible_thread::is_interruption_flag_set() ||
							(task = task_queue.pop()) != nullptr;
					});
				}

				while (task != nullptr) {
					if (interruptible_thread::is_interruption_flag_set())
						return;

					// Call task lambda
					(*task)();
					task = task_queue.pop();
				}
			}
		});
	}

	void prune_submitted_batches() {
		// Remove submitted batches from front of the list if they are finished
		decltype(submitted_batches)::iterator it;
		while ((it = submitted_batches.begin()) != submitted_batches.end()) {
			auto& f = *it->get_fence();
			if (!f->is_signaled())
				break;

			f.get();

			submitted_batches.pop_front();
		}
	}

	template <typename R, typename L>
	static auto execute_in_place(L &&task,
								 typename std::enable_if<std::is_void<R>::value>::type* = nullptr) {
		std::promise<R> promise;
		auto future = promise.get_future();

		task();
		promise.set_value();

		return future;
	}
	template <typename R, typename L>
	static auto execute_in_place(L &&task,
								 typename std::enable_if<!std::is_void<R>::value>::type* = nullptr) {
		std::promise<R> promise;
		auto future = promise.get_future();
		promise.set_value(task());

		return future;
	}

public:
	ste_device_queue(const vk_logical_device &device,
					 std::uint32_t device_family_index,
					 ste_queue_descriptor descriptor,
					 std::uint32_t queue_index,
					 ste_resource_pool<fence<void>> *fence_pool)
		: queue_index(queue_index),
		queue(device, descriptor.family, device_family_index),
		descriptor(descriptor),
		fence_pool(fence_pool),
		pool(device, descriptor.family)
	{
		// Create the queue worker thread
		create_worker();
	}
	~ste_device_queue() noexcept {
		thread->interrupt();

		do { notifier.notify_all(); } while (!m.try_lock());
		m.unlock();

		thread->join();
		queue.wait_idle();
	}

	ste_device_queue(ste_device_queue &&q) = delete;
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
	*			Must be called from an enqueued task.
	*/
	auto allocate_batch() {
		return ste_device_queue_batch(queue_index,
									  pool.claim(),
									  fence_pool->claim());
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

		if (is_queue_thread()) {
			// Execute in place
			return execute_in_place<R>(std::move(task));
		}

		// Enqueue
		enqueue_task_t<R> f(std::forward<L>(task));
		auto future = f.get_future();

		task_queue.push(std::move(f));
		notifier.notify_one();

		return future;
	}

	/**
	*	@brief	Waits idly for the queue to finish processing
	*			
	*	@throws	ste_device_exception	If thread is a queue thread
	*/
	void wait_idle() const {
		if (is_queue_thread()) {
			throw ste_device_exception("Deadlock");
		}

		std::atomic_thread_fence(std::memory_order_acquire);
		while (!task_queue.is_empty_hint()) {
			notifier.notify_all();
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}

		// And wait idle (for weired implementations that would signal the fence before the queue is complete)
		queue.wait_idle();
	}

	auto &device_queue() const { return queue; }
	auto &queue_descriptor() const { return descriptor; }
};

}
}
