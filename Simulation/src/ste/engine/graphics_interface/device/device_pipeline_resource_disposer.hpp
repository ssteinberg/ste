//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline_resources_marked_for_deletion.hpp>

#include <interruptible_thread.hpp>
#include <mutex>
#include <condition_variable>
#include <lib/unique_ptr.hpp>
#include <chrono>
#include <aligned_ptr.hpp>

#include <lib/list.hpp>
#include <lib/concurrent_queue.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	A queue that disposes of pipeline object resources. 
 *			The pipeline object and its resources might still be in use when disposed, however facilities that track the relevant fences' status are lacking. In practice those are rarely 
 *			needed and a short delay before deletion is sufficient and more performant.
 *			
 *			TODO: Better heuristics
 */
template <
	std::size_t dealloc_max_delay_ms
>
class device_pipeline_resource_disposer {
private:
	using Instance = device_pipeline_resources_marked_for_deletion;
	using queue_t = lib::concurrent_queue<Instance>;
	using instance_ptr = queue_t::stored_ptr;

	struct shared_data_t {
		mutable std::condition_variable notifier;

		queue_t queue;
	};

private:
	mutable std::mutex m;
	aligned_ptr<shared_data_t> shared_data;
	lib::unique_ptr<interruptible_thread> thread;

	lib::list<std::pair<Instance, std::chrono::high_resolution_clock::time_point>> deletion_list;

public:
	device_pipeline_resource_disposer() {
		thread = lib::allocate_unique<interruptible_thread>([this]() {
			for (;;) {
				instance_ptr queued_instance;
				{
					std::unique_lock<std::mutex> l(m);
					shared_data->notifier.wait(l, [&]() {
						return interruptible_thread::is_interruption_flag_set() ||
							(queued_instance = shared_data->queue.pop()) != nullptr;
					});
				}

				// First delete pending instances which passed their timeout
				auto now = std::chrono::high_resolution_clock::now();
				while (deletion_list.size()) {
					auto tp = deletion_list.begin()->second;
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - tp).count();
					if (ms < dealloc_max_delay_ms)
						break;

					{
						auto temp = std::move(deletion_list.begin()->first);
					}
					deletion_list.pop_front();
				}

				// Then pop queue elements and insert them into the list
				while (queued_instance != nullptr) {
					deletion_list.push_front(std::make_pair(std::move(*queued_instance), 
															std::chrono::high_resolution_clock::now()));
					queued_instance = shared_data->queue.pop();
				}

				if (interruptible_thread::is_interruption_flag_set()) {
					// Create a release->acquire barrier with the destructor
					std::atomic_thread_fence(std::memory_order_release);
					return;
				}
			}
		});
	}
	~device_pipeline_resource_disposer() noexcept {
		thread->interrupt();

		do { shared_data->notifier.notify_all(); } while (!m.try_lock());
		m.unlock();

		thread->join();

		// Wrap up
		std::atomic_thread_fence(std::memory_order_acquire);
		for (auto &p : deletion_list)
			auto temp = std::move(p.first);
		auto queued_instance = shared_data->queue.pop();
		while (queued_instance != nullptr) {
			{
				auto temp = std::move(*queued_instance);
			}
			queued_instance = shared_data->queue.pop();
		}
	}

	/**
	 *	@brief	Queue instance deletion
	 */
	void queue_deletion(Instance &&ptr) {
		shared_data->queue.push(std::move(ptr));
		shared_data->notifier.notify_one();
	}
};

}
}
