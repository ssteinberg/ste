// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <future>
#include <functional>
#include <exception>

#include <function_wrapper.hpp>
#include <function_traits.hpp>

namespace ste {

namespace _detail {

template <typename R, typename ... Params>
struct thread_pool_task_exec_impl {
	template <typename F>
	void operator()(F &f, std::promise<R> &promise, Params&&... params) const {
		auto r = f(std::forward<Params>(params)...);
		promise.set_value(std::move(r));
	}
};

template <typename ... Params>
struct thread_pool_task_exec_impl<void, Params...> {
	template <typename F>
	void operator()(F &f, std::promise<void> &promise, Params&&... params) const {
		f(std::forward<Params>(params)...);
		promise.set_value();
	}
};

}

template <typename ... Params>
class unique_thread_pool_type_erased_task {
private:
	unique_function_wrapper<Params...> callable;

protected:
	unique_thread_pool_type_erased_task() {}

	template <typename F>
	void set_callable(F &&f) {
		callable = unique_function_wrapper<Params...>(std::move(f));
	}

public:
	unique_thread_pool_type_erased_task(unique_thread_pool_type_erased_task &&) = default;
	unique_thread_pool_type_erased_task &operator=(unique_thread_pool_type_erased_task &&) = default;
	virtual ~unique_thread_pool_type_erased_task() {}

	void operator()(Params&&... params) const {
		callable(std::forward<Params>(params)...);
	}
};

template <typename R, typename ... Params>
class unique_thread_pool_task : public unique_thread_pool_type_erased_task<Params...> {
	using Base = unique_thread_pool_type_erased_task<Params...>;

private:
	std::future<R> future;

public:
	template <typename F>
	unique_thread_pool_task(F &&f) {
		static_assert(function_traits<F>::arity == sizeof...(Params), "lambda takes wrong number of arguments");

		std::promise<R> promise;
		future = promise.get_future();

		Base::set_callable([f = std::forward<F>(f), promise = std::move(promise)](Params&&... params) mutable {
			try {
				_detail::thread_pool_task_exec_impl<R, Params...>()(f, promise, std::forward<Params>(params)...);
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
