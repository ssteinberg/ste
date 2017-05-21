// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <future>
#include <functional>
#include <exception>

#include <function_wrapper.hpp>
#include <function_traits.hpp>
#include <type_traits>
#include <functor.hpp>

namespace ste {

namespace _detail {

template <typename R, typename ... Params>
struct thread_pool_task_exec_impl {
	template <typename F>
	void operator()(F &f, std::promise<R> &promise, Params&&... params) const {
		static_assert(std::is_callable_v<F(Params...)>, "F not a valid functor accepting Params...");

		auto r = f(std::forward<Params>(params)...);
		promise.set_value(std::move(r));
	}
};

template <typename ... Params>
struct thread_pool_task_exec_impl<void, Params...> {
	template <typename F>
	void operator()(F &f, std::promise<void> &promise, Params&&... params) const {
		static_assert(std::is_callable_v<F(Params...)>, "F not a valid functor accepting Params...");

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
		static_assert(std::is_callable_v<F(Params...)>, "F not a valid functor accepting Params...");
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

	virtual ~unique_thread_pool_task() noexcept {}

	auto get_future() { return std::move(future); }
};

template <typename R>
class thread_pool_task : public functor<> {
private:
	unique_function_wrapper<> task;
	std::future<R> future;

	std::atomic<std::uint8_t> executed{ 0 };

public:
	template <typename F>
	thread_pool_task(F &&f) {
		static_assert(std::is_callable_v<F()>, "F not a valid functor of arity 0");
		static_assert(std::is_void_v<R> || std::is_constructible_v<R, std::result_of_t<F()>>, "Result of F is not convetible to R");

		std::promise<R> promise;
		future = promise.get_future();

		task = unique_function_wrapper<>([f = std::forward<F>(f), promise = std::move(promise)]() mutable {
			try {
				_detail::thread_pool_task_exec_impl<R>()(f, promise);
			}
			catch (...) {
				promise.set_exception(std::current_exception());
			}
		});
	}

	~thread_pool_task() noexcept {}

	auto get_future() { return std::move(future); }

	void operator()() override final {
		// Only the first one here executes the task
		std::uint8_t expected = 0;
		if (!executed.compare_exchange_strong(expected, 0xFF, std::memory_order_acq_rel, std::memory_order_relaxed))
			return;

		task();
	}
};

}
