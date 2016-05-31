// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <memory>
#include <chrono>
#include <future>
#include <thread>
#include <list>

#include <type_traits>

#include "task_future.hpp"

#include "balanced_thread_pool.hpp"
#include "concurrent_queue.hpp"
#include "function_wrapper.hpp"
#include "function_traits.hpp"
#include "thread_constants.hpp"

namespace StE {

class task_scheduler {
private:
	using LoadBalancingPool = balanced_thread_pool;

	struct delayed_task {
		std::chrono::high_resolution_clock::time_point run_at;
		function_wrapper f;
	};

private:
	LoadBalancingPool pool;

	concurrent_queue<function_wrapper> main_thread_task_queue;
	concurrent_queue<delayed_task> delayed_tasks_queue;
	std::list<delayed_task> delayed_tasks_list;

	void enqueue_delayed();

public:
	task_scheduler() = default;
	task_scheduler(const task_scheduler &) = delete;
	task_scheduler &operator=(const task_scheduler &) = delete;
	task_scheduler(task_scheduler &&) = delete;
	task_scheduler &operator=(task_scheduler &&) = delete;

	void run_loop();

	template <bool shared, typename F>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_now(F &&f) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");

		return { std::move(pool.enqueue(std::forward<F>(f))), this };
	}

	template <bool shared, typename F>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_at(const std::chrono::high_resolution_clock::time_point &at,
												   				   				F &&f) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");

		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		delayed_tasks_queue.push({ at, std::move(pt) });
		return { std::move(future), this };
	}

	template <bool shared, typename F, class Rep, class Period>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_after(const std::chrono::duration<Rep, Period> &after,
																	  			   F &&f) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");

		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		delayed_tasks_queue.push({ std::chrono::high_resolution_clock::now() + after, std::move(pt) });
		return { std::move(future), this };
	}

	template <bool shared, typename F>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_now_on_main_thread(F &&f) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");

		std::packaged_task<std::result_of_t<F()>()> pt(std::forward<F>(f));
		auto future = pt.get_future();
		if (is_main_thread()) {
			pt();
			return { std::move(future), this };
		}

		main_thread_task_queue.push(std::move(pt));
		return { std::move(future), this };
	}

	template <typename F>
	auto schedule_now(F &&f) {
		return schedule_now<false>(std::forward<F>(f));
	}
	template <typename F>
	auto schedule_at(const std::chrono::high_resolution_clock::time_point &at, F &&f) {
		return schedule_at<false>(at, std::forward<F>(f));
	}
	template <typename F, class Rep, class Period>
	auto schedule_after(const std::chrono::duration<Rep, Period> &after, F &&f) {
		return schedule_after<false>(after, std::forward<F>(f));
	}
	template <typename F>
	auto schedule_now_on_main_thread(F &&f) {
		return schedule_now_on_main_thread<false>(std::forward<F>(f));
	}

	const balanced_thread_pool *get_thread_pool() const { return &pool; }
};

}

#include "task_future_impl.hpp"
