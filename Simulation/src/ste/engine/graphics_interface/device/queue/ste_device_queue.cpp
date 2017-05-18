
#include <stdafx.hpp>
#include <ste_device_queue.hpp>

using namespace ste::gl;

thread_local ste_device_queue *ste_device_queue::static_device_queue_ptr = nullptr;
thread_local const vk::vk_queue *ste_device_queue::static_queue_ptr = nullptr;
thread_local ste_device_queue::queue_index_t ste_device_queue::static_queue_index = 0xFFFFFFFF;

void ste_device_queue::create_worker() {
	auto idx = queue_index;
	thread = std::make_unique<interruptible_thread>([this, idx]() {
		// Set the thread_local globals to this thread's parameters
		ste_device_queue::static_device_queue_ptr = this;
		ste_device_queue::static_queue_ptr = &this->queue;
		ste_device_queue::static_queue_index = idx;

		for (;;) {
			if (interruptible_thread::is_interruption_flag_set()) return;

			task_queue_t::stored_ptr task;
			{
				std::unique_lock<std::mutex> l(shared_data->m);
				if (interruptible_thread::is_interruption_flag_set()) return;

				shared_data->notifier.wait(l, [&]() {
					return interruptible_thread::is_interruption_flag_set() ||
						(task = shared_data->task_queue.pop()) != nullptr;
				});

				while (task != nullptr) {
					if (interruptible_thread::is_interruption_flag_set())
						return;

					// Call task lambda
					(*task)();
					task = shared_data->task_queue.pop();
				}
			}
		}
	});
}

void ste_device_queue::prune_submitted_batches() {
	// Remove submitted batches from front of the list if they are finished
	decltype(submitted_batches)::iterator it;
	while ((it = submitted_batches.begin()) != submitted_batches.end()) {
		if (!(*it)->is_batch_complete())
			break;

		submitted_batches.pop_front();
	}
}
