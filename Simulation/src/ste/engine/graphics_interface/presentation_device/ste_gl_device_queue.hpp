//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_gl_device_queues_protocol.hpp>

#include <vk_queue.hpp>
#include <vk_logical_device.hpp>
#include <vk_command_pool.hpp>

#include <concurrent_queue.hpp>

#include <mutex>
#include <condition_variable>
#include <interruptible_thread.hpp>
#include <thread_pool_task.hpp>

namespace StE {
namespace GL {

class ste_gl_device_queue {
private:
	vk_queue queue;
	ste_gl_queue_descriptor descriptor;
	vk_command_pool pool;

	mutable std::mutex m;
	std::condition_variable notifier;
	std::unique_ptr<interruptible_thread> thread;

	concurrent_queue<unique_thread_pool_type_erased_task> task_queue;

public:
	ste_gl_device_queue(const vk_logical_device &device, ste_gl_queue_descriptor descriptor)
		: queue(device, descriptor.family, 0),
		descriptor(descriptor), 
		pool(device, descriptor.family)
	{
		// Create the queue worker thread
		thread = std::make_unique<interruptible_thread>([this]() {
			for (;;) {
				if (interruptible_thread::is_interruption_flag_set()) return;

				std::unique_ptr<unique_thread_pool_type_erased_task> task;
				{
					std::unique_lock<std::mutex> l(this->m);

					this->notifier.wait(l, [&]() {
						return interruptible_thread::is_interruption_flag_set() ||
							(task = task_queue.pop()) != nullptr;
					});
				}

				while (task != nullptr) {
					(*task)();
					if (interruptible_thread::is_interruption_flag_set())
						return;
					task = task_queue.pop();
				}
			}
		});
	}
	~ste_gl_device_queue() noexcept {
		thread->interrupt();
		do { notifier.notify_all(); } while (!m.try_lock());
		thread->join();
	}

	ste_gl_device_queue(ste_gl_device_queue &&q) = delete;
	ste_gl_device_queue &operator=(ste_gl_device_queue &&) = delete;
	ste_gl_device_queue(const ste_gl_device_queue &) = delete;
	ste_gl_device_queue &operator=(const ste_gl_device_queue &) = delete;

	template <typename R>
	std::future<R> enqueue(unique_thread_pool_task<R> &&f) {
		auto future = f.get_future();

		task_queue.push(std::move(f));
		notifier.notify_one();

		return future;
	}

	auto &device_queue() const { return queue; }
	auto &queue_descriptor() const { return descriptor; }
	auto &command_pool() const { return pool; }
};

}
}
