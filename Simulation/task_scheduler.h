// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "balancing_thread_pool.h"
#include "concurrent_queue.h"
#include "function_wrapper.h"
#include "function_traits.h"
#include "thread_constants.h"

#include "task.h"

#include <memory>
#include <chrono>
#include <future>
#include <thread>
#include <list>

#include <type_traits>

namespace StE {

class task_scheduler {
private:
	using LoadBalancingPool = balancing_thread_pool;

	struct delayed_task {
		std::chrono::high_resolution_clock::time_point run_at;
		function_wrapper f;
	};

private:
	LoadBalancingPool pool;

	concurrent_queue<function_wrapper> main_thread_task_queue;
	concurrent_queue<delayed_task> delayed_tasks_queue;
	std::list<delayed_task> delayed_tasks_list;

	void enqueue_delayed() {
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

public:
	task_scheduler() = default;
	task_scheduler(const task_scheduler &) = delete;
	task_scheduler &operator=(const task_scheduler &) = delete;
	task_scheduler(task_scheduler &&) = delete;
	task_scheduler &operator=(task_scheduler &&) = delete;

	void run_loop() {
		assert(is_main_thread());

		std::unique_ptr<function_wrapper> task;
		while ((task = main_thread_task_queue.pop()) != nullptr)
			(*task)();

		pool.load_balance();
		enqueue_delayed();
	}

	template <typename F>
	std::future<function_traits<F>::result_t> schedule_now(F &&f,
													std::enable_if_t<function_traits<F>::arity == 0>* = 0) {
		return pool.enqueue(std::forward<F>(f));
	}
	template <typename F>
	std::future<function_traits<F>::result_t> schedule_now(F &&f,
													std::enable_if_t<function_traits<F>::arity == 1>* = 0) {
		static_assert(std::is_constructible<function_traits<F>::arg<0>::t, task_scheduler*>::value, "Lambda argument must be constructible with task_scheduler*");

		return schedule_now([func = std::forward<F>(f), this]() { return func(this); });
	}

	template<typename F>
	std::future<function_traits<F>::result_t> schedule_at(const std::chrono::high_resolution_clock::time_point &at,
												   F &&f,
												   std::enable_if_t<function_traits<F>::arity == 0>* = 0) {
		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		delayed_tasks_queue.push({ at, std::move(pt) });
		return future;
	}
	template <typename F>
	std::future<function_traits<F>::result_t> schedule_at(const std::chrono::high_resolution_clock::time_point &at,
												   F &&f,
												   std::enable_if_t<function_traits<F>::arity == 1>* = 0) {
		static_assert(std::is_constructible<function_traits<F>::arg<0>::t, task_scheduler*>::value, "Lambda argument must be constructible with task_scheduler*");

		return schedule_at(at, [func = std::forward<F>(f), this]() { return func(this); });
	}

	template<typename F, class Rep, class Period>
	std::future<function_traits<F>::result_t> schedule_after(const std::chrono::duration<Rep, Period> &after,
													  F &&f,
													  std::enable_if_t<function_traits<F>::arity == 0>* = 0) {
		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		delayed_tasks_queue.push({ std::chrono::high_resolution_clock::now() + after, std::move(pt) });
		return future;
	}
	template <typename F, class Rep, class Period>
	std::future<function_traits<F>::result_t> schedule_after(const std::chrono::duration<Rep, Period> &after,
													  F &&f,
													  std::enable_if_t<function_traits<F>::arity == 1>* = 0) {
		static_assert(std::is_constructible<function_traits<F>::arg<0>::t, task_scheduler*>::value, "Lambda argument must be constructible with task_scheduler*");

		return schedule_after(after, [func = std::forward<F>(f), this]() { return func(this); });
	}

	template <typename F>
	std::future<function_traits<F>::result_t> schedule_now_on_main_thread(F &&f,
																   std::enable_if_t<function_traits<F>::arity == 0>* = 0) {
		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		if (is_main_thread()) {
			pt();
			return future;
		}

		main_thread_task_queue.push(std::move(pt));
		return future;
	}
	template <typename F>
	std::future<function_traits<F>::result_t> schedule_now_on_main_thread(F &&f,
																   std::enable_if_t<function_traits<F>::arity == 1>* = 0) {
		static_assert(std::is_constructible<function_traits<F>::arg<0>::t, task_scheduler*>::value, "Lambda argument must be constructible with task_scheduler*");

		return schedule_now_on_main_thread([func = std::forward<F>(f), this]() { func(this); });
	}
};

}
