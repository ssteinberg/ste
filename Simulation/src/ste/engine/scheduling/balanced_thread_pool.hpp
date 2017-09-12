// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <functor.hpp>
#include <thread_pool_task.hpp>

#include <lib/concurrent_queue.hpp>
#include <interruptible_thread.hpp>

#include <thread_constants.hpp>
#include <thread_priority.hpp>
#include <thread_affinity.hpp>

#include <system_times.hpp>

#include <atomic>
#include <mutex>
#include <future>
#include <condition_variable>
#include <lib/aligned_padded_ptr.hpp>

#include <lib/vector.hpp>
#include <lib/shared_ptr.hpp>
#include <bitset>
#include <atomic>
#include <chrono>

namespace ste {

class balanced_thread_pool {
public:
	using task_t = lib::shared_ptr<functor<>>;
	using task_queue_t = lib::concurrent_queue<task_t>;

private:
	struct shared_data_t {
		mutable std::mutex m;
		std::condition_variable notifier;

		std::atomic<std::uint32_t> requests_pending{ 0 };
		std::atomic<std::uint32_t> active_workers{ 0 };
	};

private:
	static thread_local bool balanced_thread_pool_worker_thread_flag;

public:
	static bool is_thread_pool_worker_thread() { return balanced_thread_pool_worker_thread_flag; }

private:
	lib::aligned_padded_ptr<shared_data_t> shared_data;
	lib::vector<interruptible_thread> workers;
	lib::vector<interruptible_thread> despawned_workers;

	task_queue_t task_queue;

	system_times sys_times;
	std::chrono::high_resolution_clock::time_point last_pool_balance;

	float idle_time_threshold_for_new_worker;
	float kernel_time_thershold_for_despawn_extra_worker;
	float idle_time_threshold_for_despawn_surplus_worker;

private:
	void spawn_worker(int schedule_on_cpu = -1) {
		workers.emplace_back([this]() {
			// Set balanced thread pool worker flag for this thread
			balanced_thread_pool::balanced_thread_pool_worker_thread_flag = true;

			for (;;) {
				if (interruptible_thread::is_interruption_flag_set()) return;

				task_queue_t::stored_ptr task;
				{
					// Wait for tasks
					std::unique_lock<std::mutex> l(shared_data->m);
					if (interruptible_thread::is_interruption_flag_set()) return;

					shared_data->notifier.wait(l,
											   [&]() {
											   return interruptible_thread::is_interruption_flag_set() ||
												   (task = task_queue.pop()) != nullptr;
										   });
				}

				shared_data->active_workers.fetch_add(1);

				// Process tasks
				while (task != nullptr) {
					run_task(std::move(*task));
					if (interruptible_thread::is_interruption_flag_set()) {
						shared_data->active_workers.fetch_add(-1);
						return;
					}
					task = task_queue.pop();
				}

				shared_data->active_workers.fetch_add(-1);
			}
		});

		const auto t = &workers.back().get_thread();

		thread_set_priority_low(t);
		if (schedule_on_cpu >= 0) {
			constexpr auto bits = sizeof(std::size_t) * 8;

			std::bitset<bits> mask(0);
			mask[schedule_on_cpu] = true;
			thread_set_affinity<bits>(t, mask);
		}
	}

	void despawn_worker() {
		assert(workers.size());

		auto ref = std::move(workers.back());
		workers.pop_back();

		ref.interrupt();
		ref.detach();
		despawned_workers.push_back(std::move(ref));
	}

	void notify_workers_on_enqueue() {
		const int pending = shared_data->requests_pending.fetch_add(1);
		if (pending == 0)
			shared_data->notifier.notify_one();
		else
			shared_data->notifier.notify_all();
	}

	void run_task(task_t &&task) {
		shared_data->requests_pending.fetch_add(-1, std::memory_order_release);
		(*task)();
	}

	unsigned min_worker_threads() const {
		assert(std::thread::hardware_concurrency());
		return std::max<unsigned>(std::thread::hardware_concurrency(), 1u);
	}

	unsigned max_worker_threads() const {
		assert(std::thread::hardware_concurrency());
		return std::max<unsigned>(std::thread::hardware_concurrency() * 4, 1u);
	}

public:
	balanced_thread_pool(float idle_time_threshold_for_new_worker = .1f, float kernel_time_thershold_for_despawn_extra_worker = .1f, float idle_time_threshold_for_despawn_surplus_worker = .15f)
		: idle_time_threshold_for_new_worker(idle_time_threshold_for_new_worker),
		  kernel_time_thershold_for_despawn_extra_worker(kernel_time_thershold_for_despawn_extra_worker),
		  idle_time_threshold_for_despawn_surplus_worker(idle_time_threshold_for_despawn_surplus_worker) {
		const int threads = min_worker_threads();
		const int max_threads = std::thread::hardware_concurrency();
		for (int i = 0; i < threads; ++i)
			spawn_worker(max_threads - threads + i);
	}

	~balanced_thread_pool() {
		for (auto &t : workers)
			t.interrupt();

		do { shared_data->notifier.notify_all(); }
		while (!shared_data->m.try_lock());
		shared_data->m.unlock();

		for (auto &t : workers)
			t.join();
		for (auto &t : despawned_workers)
			t.get_future().get();
	}

	balanced_thread_pool(balanced_thread_pool &&) = delete;
	balanced_thread_pool(const balanced_thread_pool &) = delete;
	balanced_thread_pool &operator=(balanced_thread_pool &&) = delete;
	balanced_thread_pool &operator=(const balanced_thread_pool &) = delete;

	/*
	 *	@brief	Enqueues a task
	 */
	template <typename R>
	std::future<R> enqueue(const lib::shared_ptr<thread_pool_task<R>> &f) {
		auto future = f->get_future();

		auto copy = f;
		task_queue.push(copy);

		notify_workers_on_enqueue();

		return future;
	}

	/*
	 *	@brief	Enqueues a task
	 */
	template <typename R>
	std::future<R> enqueue(lib::shared_ptr<thread_pool_task<R>> &&f) {
		auto future = f->get_future();
		task_queue.push(std::move(f));

		notify_workers_on_enqueue();

		return future;
	}

	auto get_workers_count() const { return workers.size(); }
	auto get_pending_requests_count() const { return shared_data->requests_pending.load(std::memory_order_acquire); }
	auto get_active_workers_count() const { return shared_data->active_workers.load(std::memory_order_acquire); }

	auto get_sleeping_workers_count() const {
		auto count = static_cast<std::int32_t>(workers.size() - get_active_workers_count());
		return static_cast<std::uint32_t>(std::max<std::int32_t>(0, count));
	}

	/**
	*	@brief	Checks if all requests have finished processing
	*/
	bool is_idle() const {
		return get_pending_requests_count() == 0 && task_queue.is_empty_hint();
	}

	/*
	 *	@brief	Load balances the pool.
	 *			Creates/destroys workers, as needed.
	 */
	void load_balance() {
		assert(is_main_thread());

		const auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_since_last_pool_balance = now - last_pool_balance;
		if (time_since_last_pool_balance.count() < .05f)
			return;
		last_pool_balance = now;

		float idle_frac = .0f;
		float kernel_frac = .0f;
		float user_frac = .0f;
		if (!sys_times.get_times_since_last_call(idle_frac, kernel_frac, user_frac))
			return;

		// Cleanup despawn queue
		for (auto it = despawned_workers.begin(); it != despawned_workers.end();) {
			if (it->get_future().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				it->get_future().get(); // Might throw
				it = despawned_workers.erase(it);
			}
			else
				++it;
		}

		// Check for workers with exceptions
		for (auto it = workers.begin(); it != workers.end();) {
			if (it->get_future().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				it->get_future().get();
				it = workers.erase(it);
			}
			else
				++it;
		}

		const unsigned min_threads = min_worker_threads();
		const auto req = get_pending_requests_count();
		const auto threads_sleeping = get_sleeping_workers_count();
		const auto total_workers_count = get_workers_count();
		if (threads_sleeping == 0 &&
			idle_frac > idle_time_threshold_for_new_worker &&
			total_workers_count < max_worker_threads()) {
			spawn_worker();
		}
		else if (workers.size() > min_threads &&
			(kernel_frac > kernel_time_thershold_for_despawn_extra_worker ||
				(req == 0 && idle_frac > idle_time_threshold_for_despawn_surplus_worker) ||
				threads_sleeping > 1)) {
			despawn_worker();
		}
	}
};

}
