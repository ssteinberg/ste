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

#include <concurrent_queue.hpp>

#include <mutex>
#include <future>
#include <condition_variable>
#include <functional>
#include <interruptible_thread.hpp>
#include <thread_pool_task.hpp>

namespace StE {
namespace GL {

class ste_gl_device_queue {
public:
	using task_t = unique_thread_pool_type_erased_task<const std::uint32_t&>;
	template <typename R>
	using enqueue_task_t = unique_thread_pool_task<R, const std::uint32_t&>;

private:
	using task_pair_t = std::pair<task_t, std::uint32_t>;

private:
	vk_queue queue;
	ste_gl_queue_descriptor descriptor;
	vk_command_pool pool;
	vk_command_buffers buffers;
	std::vector<vk_fence> fences;

	mutable std::mutex m;
	std::condition_variable notifier;
	std::unique_ptr<interruptible_thread> thread;

	concurrent_queue<task_pair_t> task_queue;

	std::uint32_t tick_count{ 0 };

private:
	static thread_local vk_command_buffers *static_command_buffers_ptr;
	static thread_local vk_queue *static_queue_ptr;
	static thread_local std::uint32_t static_queue_index;

public:
	static auto& thread_command_buffers() { return *static_command_buffers_ptr; }
	static auto& thread_queue() { return *static_queue_ptr; }
	static auto thread_queue_index() { return static_queue_index; }

private:
	std::uint32_t buffer_index() const {
		return tick_count % buffers.size();
	}

	/**
	*	@brief	Resets the command buffer
	*			Might stall if command buffer is still executing on the device
	*
	*	@param	release		Release command buffer resources in addition to reset
	*/
	void reset(bool release = false) {
		std::promise<void> buffer_available_promise;
		auto future = buffer_available_promise.get_future();

		enqueue<void>([this, release, &buffer_available_promise](std::uint32_t idx) -> void {
			auto &fence = fences[idx];
			auto &buffer = buffers[idx];

			// Wait for fence and signal buffer available
			fence.wait_idle();
			buffer_available_promise.set_value();

			// Reset fence and buffer
			fence.reset();
			if (!release)
				buffer.reset();
			else
				buffer.reset_release();
		});

		// Wait for buffer to become available before returning
		future.wait();
	}

public:
	ste_gl_device_queue(const vk_logical_device &device, 
						ste_gl_queue_descriptor descriptor,
						std::uint32_t buffers_count,
						std::uint32_t queue_index)
		: queue(device, descriptor.family, 0),
		descriptor(descriptor), 
		pool(device, descriptor.family),
		buffers(pool.allocate_buffers(buffers_count))
	{
		// Create the fences in signaled state
		for (int i = 0; i < buffers_count; ++i)
			fences.emplace_back(device, true);

		// Create the queue worker thread
		thread = std::make_unique<interruptible_thread>([this, queue_index]() {
			// Set the thread_local global to this thread's parameters
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
		queue.wait_idle();
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

		// Reset current command buffer
		// Once in a while release command buffer resources to avoid command buffers growing indefinitely
		bool release = tick_count % 1000 == 0;
		reset(release);
	}

	/**
	*	@brief	Enqueues a task on the queue's thread
	*
	*	@param	f		Lambda expression
	*/
	template <typename R>
	std::future<R> enqueue(enqueue_task_t<R> &&f) {
		auto future = f.get_future();

		task_queue.push(std::make_pair(std::move(f), buffer_index()));
		notifier.notify_one();

		return future;
	}

	/**
	*	@brief	Enqueues a submit task that submits the command buffer to the queue
	*
	*	@param	wait_semaphores		See vk_queue::submit
	*	@param	signal_semaphores	See vk_queue::submit
	*/
	auto submit(const std::vector<std::pair<vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
				const std::vector<vk_semaphore*> &signal_semaphores) {
		return enqueue<void>([=](std::uint32_t idx) -> void {
			auto &fence = fences[idx];
			auto &buffer = buffers[idx];

			queue.submit({ &buffer },
						 wait_semaphores,
						 signal_semaphores,
						 &fence);
		});
	}

	auto &device_queue() const { return queue; }
	auto &queue_descriptor() const { return descriptor; }
	auto &get_command_buffers() const { return buffers; }
};

}
}
