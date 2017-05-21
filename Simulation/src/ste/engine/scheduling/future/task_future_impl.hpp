// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <task_future.hpp>
#include <task_scheduler.hpp>

#include <thread>

namespace ste {

namespace _detail {

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then(task_future_impl<R, is_shared> &&f, task_scheduler *sched, 
																		L &&lambda,
															 		 	std::enable_if_t<!std::is_void<R>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 1, "lambda must take 1 argument");

	return sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		R r = f.get();
		return func(std::move(r));
	});
}

template <typename R, bool is_shared, typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> then(task_future_impl<R, is_shared> &&f, task_scheduler *sched, 
																		L &&lambda,
															 		 	std::enable_if_t<std::is_void<R>::value>* = nullptr)  {
	static_assert(function_traits<L>::arity == 0, "lambda can not take any arguments");

	return sched->schedule_now<is_shared>([func = std::forward<L>(lambda), f = std::move(f), sched = sched]() mutable {
		f.get();
		return func();
	});
}

}

template <typename R, bool is_shared>
template <typename L>
task_future_impl<typename function_traits<L>::result_t, is_shared> task_future_impl<R, is_shared>::then(L &&lambda) && {
	return _detail::then<R, is_shared>(std::move(*this), this->sched, std::forward<L>(lambda));
}

template <typename R, bool is_shared>
task_future_impl<R, true> task_future_impl<R, is_shared>::shared() && {
	return sched->schedule_now<true>([f = std::move(*this), sched = this->sched]() mutable -> R {
		return f.get();
	});
}

}
