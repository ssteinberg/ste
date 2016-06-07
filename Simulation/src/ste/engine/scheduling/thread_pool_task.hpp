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

class unique_thread_pool_type_erased_task {
private:
	unique_function_wrapper callable;

protected:
	unique_thread_pool_type_erased_task() {}

	template <typename F>
	void set_callable(F &&f) {
		callable = unique_function_wrapper(std::move(f));
	}

public:
	unique_thread_pool_type_erased_task(unique_thread_pool_type_erased_task &&) = default;
	unique_thread_pool_type_erased_task &operator=(unique_thread_pool_type_erased_task &&) = default;
	virtual ~unique_thread_pool_type_erased_task() {}

	void operator()() const {
		callable();
	}
};

template <typename R>
class unique_thread_pool_task : public unique_thread_pool_type_erased_task {
	using Base = unique_thread_pool_type_erased_task;

private:
	std::future<R> future;

public:
	template <typename F>
	unique_thread_pool_task(F &&f) {
		static_assert(function_traits<F>::arity == 0, "lambda takes too many arguments");

		std::promise<R> promise;
		future = promise.get_future();

		Base::set_callable([f = std::forward<F>(f), promise = std::move(promise)]() mutable {
			try {
				_detail::thread_pool_task_exec_impl<R>()(f, promise);
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		});
	}

	unique_thread_pool_task(unique_thread_pool_task &&) = default;
	unique_thread_pool_task &operator=(unique_thread_pool_task &&) = default;

	virtual ~unique_thread_pool_task() {}

	auto get_future() { return std::move(future); }
};

}
