//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_queues_protocol.hpp>

#include <vk_queue.hpp>
#include <vk_logical_device.hpp>
#include <vk_command_pool.hpp>
#include <vk_command_buffers.hpp>
#include <vk_fence.hpp>

#include <condition_variable>
#include <mutex>
#include <future>
#include <utility>

#include <concurrent_queue.hpp>
#include <interruptible_thread.hpp>
#include <thread_pool_task.hpp>

#include <function_traits.hpp>
#include <type_traits>

namespace StE {
namespace GL {

class ste_gl_device_queue {
public:
	using task_arg_t = std::uint32_t;
	using task_t = unique_thread_pool_type_erased_task<const task_arg_t&>;
	template <typename R>
	using enqueue_task_t = unique_thread_pool_task<R, const task_arg_t&>;

private:
	using task_pair_t = std::pair<task_t, std::uint32_t>;
	struct sync_primitives_t {
		mutable vk_fence buffer_fence;
		mutable std::promise<void> buffer_ready_promise;
		mutable std::future<void> buffer_ready_future;

		sync_primitives_t() = delete;
	};

private:
	const vk_queue queue;
	const ste_gl_queue_descriptor descriptor;

	const vk_command_pool pool;
	const vk_command_pool pool_transient;
	vk_command_buffers buffers;
	const std::vector<sync_primitives_t> sync_primitives;

	mutable std::mutex m;
	std::condition_variable notifier;
	std::unique_ptr<interruptible_thread> thread;

	concurrent_queue<task_pair_t> task_queue;

	std::uint32_t tick_count{ 0 };

private:
	static thread_local ste_gl_device_queue *static_device_queue_ptr;
	static thread_local const vk_command_pool *static_command_pool_ptr;
	static thread_local const vk_command_pool *static_command_pool_transient_ptr;
	static thread_local vk_command_buffers *static_command_buffers_ptr;
	static thread_local const vk_queue *static_queue_ptr;
	static thread_local std::uint32_t static_queue_index;

public:
	/**
	*	@brief	Returns the device queue (ste_gl_device_queue) for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto& thread_device_queue() { return *static_device_queue_ptr; }
	/**
	*	@brief	Returns the command pool for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto& thread_command_pool() { return *static_command_pool_ptr; }
	/**
	*	@brief	Returns the transient command pool for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto& thread_command_pool_transient() { return *static_command_pool_transient_ptr; }
	/**
	*	@brief	Returns the primary command buffers for the current thread
	*			Must be called from an enqueued task.
	*/
	static auto& thread_command_buffers() { return *static_command_buffers_ptr; }
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
	*	@brief	Submits the command buffer to the queue
	*			Must be called from an enqueued task.
	*
	*	@param	wait_semaphores		See vk_queue::submit
	*	@param	signal_semaphores	See vk_queue::submit
	*/
	static void submit_current_queue(const task_arg_t &buffer_idx,
									 const std::vector<std::pair<const vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
									 const std::vector<const vk_semaphore*> &signal_semaphores) {
		auto &sync = thread_device_queue().sync_primitives[buffer_idx];
		auto &buffer = thread_device_queue().buffers[buffer_idx];

		thread_queue().submit({ &buffer },
							  wait_semaphores,
							  signal_semaphores,
							  &sync.buffer_fence);

		// Wait for fence and signal buffer available
		sync.buffer_fence.wait_idle();
		sync.buffer_ready_promise.set_value();

		// Reset current command buffer and fence
		// Once in a while release command buffer resources to avoid command buffers growing indefinitely
		bool release = thread_device_queue().tick_count % 1000 == 0;
		thread_device_queue().reset(buffer_idx, release);
	}

private:
	std::uint32_t buffer_index() const {
		return tick_count % buffers.size();
	}

	static auto create_buffer_sync_primitives(const vk_logical_device &device, 
											  std::uint32_t buffers_count) {
		// Create buffer synchronization primitives
		std::vector<sync_primitives_t> v;

		v.reserve(buffers_count);
		for (int i = 0; i < buffers_count; ++i) {
			// Create the fences in unsignaled state
			sync_primitives_t sync = { vk_fence(device, false) };
			// Create buffer ready promise-future in a ready state
			sync.buffer_ready_future = sync.buffer_ready_promise.get_future();
			sync.buffer_ready_promise.set_value();

			v.push_back(std::move(sync));
		}

		return v;
	}

	/**
	*	@brief	Resets the command buffer
	*
	*	@param	idx			Index of command buffer
	*	@param	release		Release command buffer resources in addition to reset
	*/
	void reset(const task_arg_t &idx, 
			   bool release = false) {
		auto &sync = sync_primitives[idx];
		auto &buffer = buffers[idx];

		sync.buffer_fence.reset();
		if (!release)
			buffer.reset();
		else
			buffer.reset_release();
	}

public:
	ste_gl_device_queue(const vk_logical_device &device, 
						std::uint32_t device_family_index,
						ste_gl_queue_descriptor descriptor,
						std::uint32_t buffers_count,
						std::uint32_t queue_index)
		: queue(device, descriptor.family, device_family_index),
		descriptor(descriptor), 
		pool(device, descriptor.family),
		pool_transient(device, descriptor.family, true),
		buffers(pool.allocate_buffers(buffers_count)),
		sync_primitives(create_buffer_sync_primitives(device, buffers_count))
	{
		// Create the queue worker thread
		thread = std::make_unique<interruptible_thread>([this, queue_index]() {
			// Set the thread_local globals to this thread's parameters
			ste_gl_device_queue::static_device_queue_ptr = this;
			ste_gl_device_queue::static_command_pool_ptr = &this->pool;
			ste_gl_device_queue::static_command_pool_transient_ptr = &this->pool_transient;
			ste_gl_device_queue::static_command_buffers_ptr = &this->buffers;
			ste_gl_device_queue::static_queue_ptr = &this->queue;
			ste_gl_device_queue::static_queue_index = queue_index;

			for (;;) {
				if (interruptible_thread::is_interruption_flag_set()) return;

				std::unique_ptr<task_pair_t> task;
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

					// Call task lambda, passing command buffer index
					task->first(task->second);
					task = task_queue.pop();
				}
			}
		});
	}
	~ste_gl_device_queue() noexcept {
		thread->interrupt();

		do { notifier.notify_all(); } while (!m.try_lock());
		m.unlock();

		thread->join();
		wait_idle();
	}

	ste_gl_device_queue(ste_gl_device_queue &&q) = delete;
	ste_gl_device_queue &operator=(ste_gl_device_queue &&) = delete;
	ste_gl_device_queue(const ste_gl_device_queue &) = delete;
	ste_gl_device_queue &operator=(const ste_gl_device_queue &) = delete;

	/**
	*	@brief	Prepares the next command buffer
	*/
	void acquire_next_command_buffer() {
		// Increase tick count
		++tick_count;

		// Wait for buffer ready
		auto &sync = sync_primitives[buffer_index()];
		sync.buffer_ready_future.get();

		// Create buffer promise-future for this iteration
		sync.buffer_ready_promise = std::promise<void>();
		sync.buffer_ready_future = sync.buffer_ready_promise.get_future();
	}

	/**
	*	@brief	Enqueues a task on the queue's thread
	*
	*	@param	task	Task to enqueue
	*/
	template <typename L>
	std::future<typename function_traits<L>::result_t> enqueue(L &&task,
															   typename std::enable_if<function_traits<L>::arity == 1>::type* = nullptr) {
		using R = typename function_traits<L>::result_t;

		static_assert(function_traits<L>::arity == 1 &&
					  std::is_convertible<typename function_traits<L>::template arg<0>::t, const task_arg_t&>::value,
					  "task must take none or a single buffer index parameter");

		enqueue_task_t<R> f(std::forward<L>(task));
		auto future = f.get_future();

		task_queue.push(std::make_pair(std::move(f), buffer_index()));
		notifier.notify_one();

		return future;
	}
	/**
	*	@brief	Enqueues a task on the queue's thread
	*
	*	@param	task	Task to enqueue
	*/
	template <typename L>
	std::future<typename function_traits<L>::result_t> enqueue(L &&task,
															   typename std::enable_if<function_traits<L>::arity != 1>::type* = nullptr) {
		static_assert(function_traits<L>::arity == 0,
					  "task must take none or a single buffer index parameter");
		return enqueue([task = std::forward<L>(task)](const task_arg_t&) { return task(); });
	}

	/**
	*	@brief	Waits idly for the queue to finish processing
	*/
	void wait_idle() const {
		queue.wait_idle();
	}

	auto &device_queue() const { return queue; }
	auto &queue_descriptor() const { return descriptor; }
	auto &get_command_pool() const { return pool; }
	auto &get_command_pool_transient() const { return pool_transient; }
	auto &get_command_buffers() const { return buffers; }
};

}
}
