// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "task_future.hpp"
#include "task_scheduler.hpp"

namespace StE {

namespace _detail {

template <typename F, typename L>
task_future<typename function_traits<L>::result_t> then(task_future<F> &&f, task_scheduler *sched, L &&lambda,
														std::enable_if_t<!std::is_void<F>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 1, "lambda must take 1 argument");

	return sched->schedule_now([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		F r = f.get();
		return func(std::move(r));
	});
}

template <typename F, typename L>
task_future_chain<typename function_traits<L>::result_t> then_on_main_thread(task_future<F> &&f, task_scheduler *sched, L &&lambda,
																	   		 std::enable_if_t<!std::is_void<F>::value>* = nullptr) {
	static_assert(function_traits<L>::arity == 1, "lambda must take 1 argument");

	return sched->schedule_now([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable -> task_future<typename function_traits<L>::result_t> {
		F r = f.get();
		return sched->schedule_now_on_main_thread([func = std::move(func), r = std::move(r)]() mutable {
			return func(std::move(r));
		});
	});
}

template <typename F, typename L>
task_future<typename function_traits<L>::result_t> then(task_future<F> &&f, task_scheduler *sched, L &&lambda,
														std::enable_if_t<std::is_void<F>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 0, "lambda must take 0 arguments");

	return sched->schedule_now([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		f.get();
		return func();
	});
}

template <typename F, typename L>
task_future_chain<typename function_traits<L>::result_t> then_on_main_thread(task_future<F> &&f, task_scheduler *sched, L &&lambda,
																	   		 std::enable_if_t<std::is_void<F>::value>* = nullptr) {
	static_assert(function_traits<L>::arity == 0, "lambda must take 0 arguments");

	return sched->schedule_now([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable -> task_future<typename function_traits<L>::result_t> {
		f.get();
		return sched->schedule_now_on_main_thread([func = std::move(func)]() {
			return func();
		});
	});
}

}

template <typename R>
template <typename L>
task_future<typename function_traits<L>::result_t> task_future<R>::then(L &&lambda) && {
	return _detail::then<R>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R>
template <typename L>
task_future_chain<typename function_traits<L>::result_t> task_future<R>::then_on_main_thread(L &&lambda) && {
	return _detail::then_on_main_thread<R>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R>
template <typename L>
task_future<typename function_traits<L>::result_t> task_future_chain<R>::then(L &&lambda) && {
	return _detail::then<R>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R>
template <typename L>
task_future_chain<typename function_traits<L>::result_t> task_future_chain<R>::then_on_main_thread(L &&lambda) && {
	return _detail::then_on_main_thread<R>(std::move(*this), this->sched, std::forward<L>(lambda));
}

}
