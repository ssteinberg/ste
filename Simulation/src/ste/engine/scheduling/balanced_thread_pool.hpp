// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "concurrent_queue.hpp"
#include "interruptible_thread.hpp"
#include "function_wrapper.hpp"
#include "thread_constants.hpp"
#include "thread_priority.hpp"

#include "system_times.hpp"

#include <atomic>
#include <mutex>
#include <future>
#include <condition_variable>

#include <vector>
#include <chrono>

namespace StE {

class balanced_thread_pool {
private:
	mutable std::mutex m;
	std::condition_variable notifier;
	std::vector<interruptible_thread> workers;
	std::vector<std::pair<std::future<bool>, interruptible_thread>> despawned_workers;

	concurrent_queue<function_wrapper> task_queue;

	system_times sys_times;
	std::chrono::high_resolution_clock::time_point last_pool_balance;
	std::atomic<int> requests_pending{ 0 };
	int threads_sleeping{ 0 };
	
	float idle_time_threshold_for_new_worker;
	float kernel_time_thershold_for_despawn_extra_worker;
	float idle_time_threshold_for_despawn_surplus_worker;

private:
	void spawn_worker() {
		workers.emplace_back([this]() {
			std::unique_ptr<function_wrapper> task;
			auto flag = interruptible_thread::interruption_flag;

			for (;;) {
				if (flag->is_set()) return;

				{
					std::unique_lock<std::mutex> l(this->m);
					threads_sleeping++;
					this->notifier.wait(l, [&]() {
						return flag->is_set() || (task = task_queue.pop()) != nullptr;
					});
					threads_sleeping--;
				}

				while (task != nullptr) {
					run_task(task);
					if (flag->is_set()) return;
					task = task_queue.pop();
				}
			}
		});

		thread_set_priority_low(&workers.back().get_thread());
	}

	void despawn_worker() {
		assert(workers.size());
		auto &ref = workers.back();
		ref.interrupt();
		ref.detach();
		auto future = ref.get_future();
		despawned_workers.push_back(std::make_pair(std::move(future), std::move(ref)));
		workers.pop_back();
	}

	void on_enqueue() {
		requests_pending++;
		notifier.notify_one();
	}

	void run_task(const std::unique_ptr<function_wrapper> &task) {
		requests_pending--;
		(*task)();
	}
	
	unsigned min_worker_threads() const {
		assert(std::thread::hardware_concurrency());

		return std::max<unsigned>(std::thread::hardware_concurrency() - 1, 1u);
	}

public:
	balanced_thread_pool(float idle_time_threshold_for_new_worker = .1f, float kernel_time_thershold_for_despawn_extra_worker = .1f, float idle_time_threshold_for_despawn_surplus_worker = .2f)
		: idle_time_threshold_for_new_worker(idle_time_threshold_for_new_worker),
		  kernel_time_thershold_for_despawn_extra_worker(kernel_time_thershold_for_despawn_extra_worker),
		  idle_time_threshold_for_despawn_surplus_worker(idle_time_threshold_for_despawn_surplus_worker) {
		int threads = min_worker_threads();
		for (int i = 0; i < threads; ++i)
			spawn_worker();
	}

	~balanced_thread_pool() {
		for (auto &t : workers)
			t.interrupt();
		do { notifier.notify_all(); } while (!m.try_lock());
		m.unlock();
		for (auto &t : workers)
			t.join();
		for (auto &t : despawned_workers)
			t.first.wait();
	}

	balanced_thread_pool(balanced_thread_pool &&) = delete;
	balanced_thread_pool(const balanced_thread_pool &) = delete;
	balanced_thread_pool &operator=(balanced_thread_pool &&) = delete;
	balanced_thread_pool &operator=(const balanced_thread_pool &) = delete;

 	template <typename F>
 	std::future<std::result_of_t<F()>> enqueue(F &&f) {
		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
 		auto future = pt.get_future();
 		task_queue.push(std::move(pt));

		on_enqueue();

 		return future;
	}

	void load_balance() {
		assert(is_main_thread());

		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_since_last_pool_balance = now - last_pool_balance;
		if (time_since_last_pool_balance.count() < .05f)
			return;
		last_pool_balance = now;

		float idle_frac, kernel_frac, user_frac;
		if (!sys_times.get_times_since_last_call(idle_frac, kernel_frac, user_frac))
			return;

		for (auto it = despawned_workers.begin(); it != despawned_workers.end(); ) {
			if (!it->first.valid() ||
				it->first.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
				it = despawned_workers.erase(it);
			else
				++it;
		}

		unsigned min_threads = min_worker_threads();
		int req = requests_pending.load();
		if (threads_sleeping == 0 &&
			idle_frac > idle_time_threshold_for_new_worker) {
			spawn_worker();
		}
		else if (workers.size() > min_threads &&
					(kernel_frac > kernel_time_thershold_for_despawn_extra_worker || 
				  	 (req == 0 && idle_frac > idle_time_threshold_for_despawn_surplus_worker) || 
				  	 threads_sleeping > 1)) {
			despawn_worker();
		}
	}

	int get_workers_count() const { return workers.size(); }
	int get_sleeping_workers() const { return threads_sleeping; }
};

}
