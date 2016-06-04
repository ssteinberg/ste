// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <future>
#include <functional>
#include <exception>

#include "function_wrapper.hpp"
#include "function_traits.hpp"

namespace StE {

namespace _detail {

template <typename R>
struct thread_pool_task_exec_impl {
	template <typename F>
	void operator()(F &f, std::promise<R> &promise) const {
		auto r = f();
		promise.set_value(std::move(r));
	}
};

template <>
struct thread_pool_task_exec_impl<void> {
	template <typename F>
	void operator()(F &f, std::promise<void> &promise) const {
		f();
		promise.set_value();
	}
};

}

template <typename R>
class thread_pool_task {
private:
	std::promise<R> promise;
	unique_function_wrapper callable;

public:
	template <typename F>
	thread_pool_task(F &&f) : callable([f = std::forward<F>(f), this]() mutable {
		try {
			_detail::thread_pool_task_exec_impl<R>()(f, this->promise);
		}
		catch (...) {
			this->promise.set_exception(std::current_exception());
		}
	}) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");
	}

	void operator()() const { callable(); }
	auto get_future() { return promise.get_future(); }
};

}
