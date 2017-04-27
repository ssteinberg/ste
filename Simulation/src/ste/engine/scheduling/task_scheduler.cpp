
#include <stdafx.hpp>
#include <task_scheduler.hpp>
#include <task_future.hpp>

using namespace ste;

void task_scheduler::enqueue_delayed() {
	auto now = std::chrono::high_resolution_clock::now();
	for (auto it = delayed_tasks_list.begin(); it != delayed_tasks_list.end();) {
		if (now >= it->run_at) {
			schedule_now(std::move(it->f));
			it = delayed_tasks_list.erase(it);
		}
		else
			++it;
	}

	for (auto task = delayed_tasks_queue.pop(); task != nullptr; task = delayed_tasks_queue.pop()) {
		if (now >= task->run_at)
			schedule_now(std::move(task->f));
		else
			delayed_tasks_list.push_front(std::move(*task));
	}
}
