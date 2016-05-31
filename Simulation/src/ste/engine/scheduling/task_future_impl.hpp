// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "task_future.hpp"
#include "task_scheduler.hpp"

#include <thread>

namespace StE {

namespace _detail {

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then(task_future_impl<R, is_shared> &&f, task_scheduler *sched, L &&lambda,
															 		 	std::enable_if_t<!std::is_void<R>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 1, "lambda must take 1 argument");

	return sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		R r = f.get();
		return func(std::move(r));
	});
}

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then_on_main_thread(task_future_impl<R, is_shared> &&f, task_scheduler *sched, L &&lambda,
																					   std::enable_if_t<!std::is_void<R>::value>* = nullptr) {
	static_assert(function_traits<L>::arity == 1, "lambda must take 1 argument");

	return { sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable -> task_future_impl<typename function_traits<L>::result_t, is_shared> {
		R r = f.get();
		return sched->schedule_now_on_main_thread<is_shared>([func = std::move(func), r = std::move(r)]() mutable {
			return func(std::move(r));
		});
	}), task_future_chaining_construct() };
}

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then(task_future_impl<R, is_shared> &&f, task_scheduler *sched, L &&lambda,
															 		 	std::enable_if_t<std::is_void<R>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 0, "lambda must take 0 arguments");

	return sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		f.get();
		return func();
	});
}

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then_on_main_thread(task_future_impl<R, is_shared> &&f, task_scheduler *sched, L &&lambda,
																			   		   std::enable_if_t<std::is_void<R>::value>* = nullptr) {
	static_assert(function_traits<L>::arity == 0, "lambda must take 0 arguments");

	return { sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable -> task_future_impl<typename function_traits<L>::result_t, is_shared> {
		f.get();
		return sched->schedule_now_on_main_thread<is_shared>([func = std::move(func)]() {
			return func();
		});
	}), task_future_chaining_construct() };
}

}

template <typename R, bool is_shared>
void task_future_impl<R, is_shared>::loop_until_ready() const {
	while (wait_for(std::chrono::microseconds(0)) != std::future_status::ready) {
		sched->run_loop();
		std::this_thread::yield();
	}
}

template <typename R, bool is_shared>
template <typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> task_future_impl<R, is_shared>::then(L &&lambda) && {
	return _detail::then<R, is_shared>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R, bool is_shared>
template <typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> task_future_impl<R, is_shared>::then_on_main_thread(L &&lambda) && {
	return _detail::then_on_main_thread<R, is_shared>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R, bool is_shared>
task_future_impl<R, true> task_future_impl<R, is_shared>::shared() && {
	return sched->schedule_now<true>([f = std::move(*this), sched = this->sched]() mutable -> R {
		return f.get();
	});
}

}
