// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <chrono>
#include <future>

#include <type_traits>

#include <task_future.hpp>
#include <thread_pool_task.hpp>

#include <balanced_thread_pool.hpp>
#include <lib/concurrent_queue.hpp>
#include <function_traits.hpp>

#include <lib/list.hpp>
#include <lib/unique_ptr.hpp>
#include <lib/shared_ptr.hpp>
#include <anchored.hpp>

namespace ste {

/**
 *	@brief	Thread-safe concurrent, wait-free task scheduler. 
 * 			Uses a load balancing thread pool. Returns task_futures that natively interruct with task_scheduler.
 */
class task_scheduler : anchored {
private:
	using pool_t = balanced_thread_pool;

	struct delayed_task {
		std::chrono::high_resolution_clock::time_point run_at;
		lib::unique_ptr<functor<>> f;
	};

private:
	pool_t pool;

	lib::concurrent_queue<delayed_task> delayed_tasks_queue;
	lib::list<delayed_task> delayed_tasks_list;

	void enqueue_delayed();

public:
	task_scheduler() = default;
	task_scheduler(const task_scheduler &) = delete;
	task_scheduler &operator=(const task_scheduler &) = delete;
	task_scheduler(task_scheduler &&) = delete;
	task_scheduler &operator=(task_scheduler &&) = delete;

	/**
	*	@brief	Load balances thread pool and enqueues delayed tasks
	*/
	void tick() {
		pool.load_balance();
		enqueue_delayed();
	}

	/**
	*	@brief	Schedule task in background for execution as soon as a worker is free.
	*			Or, in case called from a worker thread (e.g. by another task), executes the task immediately.
	*
	*	@param f		Lambda to schedule
	*	@param shared	If true returns a task_shared_future, otherwise a task_future.
	*/
	template <bool shared, typename F>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_now(F &&f) {
		using R = typename function_traits<F>::result_t;
		auto task = lib::allocate_shared<thread_pool_task<R>>(std::forward<F>(f));
		
		auto future = pool.enqueue(task);
		
		return { std::move(future), this, lib::shared_ptr<functor<>>(std::move(task)) };
	}

	/**
	*	@brief	Schedule task in background for execution at a specifed timepoint.
	*
	*	@param at		Timepoint
	*	@param f		Lambda to schedule
	*	@param shared	If true returns a task_shared_future, otherwise a task_future.
	*/
	template <bool shared, typename F>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_at(const std::chrono::high_resolution_clock::time_point &at,
												   				   				F &&f) {
		using R = typename function_traits<F>::result_t;
		auto task = lib::allocate_shared<thread_pool_task<R>>(std::forward<F>(f));
		auto future = task->get_future();

		delayed_tasks_queue.push({ at, std::move(task) });
		return { std::move(future), this, nullptr };
	}

	/**
	*	@brief	Schedule task in background for execution after specified duration.
	*
	*	@param after	Time duration to wait before scheduling
	*	@param f		Lambda to schedule
	*	@param shared	If true returns a task_shared_future, otherwise a task_future.
	*/
	template <bool shared, typename F, class Rep, class Period>
	task_future_impl<typename function_traits<F>::result_t, shared> schedule_after(const std::chrono::duration<Rep, Period> &after,
																	  			   F &&f) {
		using R = typename function_traits<F>::result_t;
		auto task = lib::allocate_shared<thread_pool_task<R>>(std::forward<F>(f));
		auto future = task->get_future();

		delayed_tasks_queue.push({ std::chrono::high_resolution_clock::now() + after, std::move(task) });
		return { std::move(future), this, nullptr };
	}

	/**
	*	@brief	Schedule task in background for execution as soon as a worker is free.
	*
	*	@param f		Lambda to schedule
	*/
	template <typename F>
	auto schedule_now(F &&f) {
		return schedule_now<false>(std::forward<F>(f));
	}

	/**
	*	@brief	Schedule task in background for execution at a specifed timepoint.
	*
	*	@param at		Timepoint
	*	@param f		Lambda to schedule
	*/
	template <typename F>
	auto schedule_at(const std::chrono::high_resolution_clock::time_point &at, F &&f) {
		return schedule_at<false>(at, std::forward<F>(f));
	}

	/**
	*	@brief	Schedule task in background for execution after specified duration.
	*
	*	@param after	Time duration to wait before scheduling
	*	@param f		Lambda to schedule
	*/
	template <typename F, class Rep, class Period>
	auto schedule_after(const std::chrono::duration<Rep, Period> &after, F &&f) {
		return schedule_after<false>(after, std::forward<F>(f));
	}

	const balanced_thread_pool *get_thread_pool() const { return &pool; }
};

}

#include <task_future_impl.hpp>
