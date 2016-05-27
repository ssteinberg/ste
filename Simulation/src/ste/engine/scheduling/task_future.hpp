// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "function_traits.hpp"
#include "thread_constants.hpp"

#include <functional>
#include <future>
#include <type_traits>

namespace StE {

class task_scheduler;
template <typename R>
class task_future;

template <typename R>
class task_future_chain {
	friend class task_scheduler;

	using future_type = typename std::future<task_future<R>>;

private:
	task_scheduler *sched;
	future_type future;

private:
	task_future_chain(future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}

public:
	task_future_chain(task_future<task_future<R>> &&f) : sched(f.sched), future(std::move(f.future)) {}

	task_future_chain(const task_future_chain &) = default;
	task_future_chain &operator=(const task_future_chain &) = default;
	task_future_chain(task_future_chain &&) = default;
	task_future_chain &operator=(task_future_chain &&) = default;

	~task_future_chain() noexcept {}

	template <typename L>
	task_future<typename function_traits<L>::result_t> then(L &&lambda) &&;
	template <typename L>
	task_future_chain<typename function_traits<L>::result_t> then_on_main_thread(L &&lambda) &&;

	R get() {
		return future.get().get();
	}
};

template <typename R>
class task_future {
	friend class task_scheduler;
	template <typename T>
	friend class task_future_chain;

	using future_type = typename std::future<R>;

private:
	task_scheduler *sched;
	future_type future;

private:
	task_future(future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}

public:
	task_future(const task_future &) = default;
	task_future &operator=(const task_future &) = default;
	task_future(task_future &&) = default;
	task_future &operator=(task_future &&) = default;

	~task_future() noexcept {}

	template <typename L>
	task_future<typename function_traits<L>::result_t> then(L &&lambda) &&;
	template <typename L>
	task_future_chain<typename function_traits<L>::result_t> then_on_main_thread(L &&lambda) &&;

	R get() {
		assert(!is_main_thread() && "Blocking main thread");
		return future.get();
	}
};

}
