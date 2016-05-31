// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "function_traits.hpp"
#include "thread_constants.hpp"

#include "future_collection.hpp"

#include <functional>
#include <memory>

#include <future>
#include <mutex>

#include <chrono>

#include <type_traits>

namespace StE {

class task_scheduler;

class task_future_chaining_construct {};

/**
 *	@brief	Wrapper around std::future for StE::task_scheduler
 *
 *	@param R			Future return type
 *	@param is_shared	Indicates whether or not this is a shared_future
*/
template <typename R, bool is_shared>
class task_future_impl {
	friend class task_scheduler;
	template <typename, bool>
	friend class task_future_impl;

public:
	using future_type = typename std::conditional<is_shared, std::shared_future<R>, std::future<R>>::type;
	using chained_task_future = task_future_impl<R, is_shared>;
	using chaining_future_type = typename std::conditional<is_shared, std::shared_future<chained_task_future>, std::future<chained_task_future>>::type;

private:
	task_scheduler *sched;
	future_type future;

	bool chain{ false };
	mutable chaining_future_type chaining_future;
	mutable std::shared_ptr<chained_task_future> chained_future{ nullptr };

	mutable std::mutex chaining_mutex;

private:
	task_future_impl(typename task_future_impl<R, false>::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}
	task_future_impl(typename task_future_impl<R, true >::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}
	template <bool b = is_shared>
	task_future_impl(const typename task_future_impl<R, true>::future_type &f,
					 task_scheduler *sched,
					 typename std::enable_if_t<b>* = nullptr) : sched(sched), future(f) {}

	void resolve_chained_future() const {
		std::unique_lock<std::mutex> ul(chaining_mutex);

		if (chained_future == nullptr)
			chained_future = std::make_shared<chained_task_future>(chaining_future.get());
	}

	auto chained_get() {
		if (chained_future == nullptr)
			resolve_chained_future();
		return chained_future->get();
	}

	void chained_wait() const {
		if (chained_future == nullptr)
			resolve_chained_future();

		chained_future->wait();
	}

	template <class Rep, class Period>
	auto chained_wait_for(const std::chrono::duration<Rep,Period> &timeout_duration) const {
		auto available_time = timeout_duration;

		if (chained_future == nullptr) {
			auto start = std::chrono::high_resolution_clock::now();

			{
				std::unique_lock<std::mutex> ul(chaining_mutex);
				if (chained_future == nullptr) {
					auto wait_result = chaining_future.wait_for(timeout_duration);
					if (wait_result == std::future_status::ready)
						chained_future = std::make_shared<chained_task_future>(chaining_future.get());
					else
						return wait_result;
				}
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<decltype(available_time)>(end - start);
			available_time -= elapsed;
		}

		return chained_future->wait_for(available_time);
	}

	template <class Clock, class Duration>
	auto chained_wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		if (chained_future == nullptr) {
			std::unique_lock<std::mutex> ul(chaining_mutex);
			if (chained_future == nullptr) {
				auto wait_result = chaining_future.wait_until(timeout_time);
				if (wait_result == std::future_status::ready)
					chained_future = std::make_shared<chained_task_future>(chaining_future.get());
				else
					return wait_result;
			}
		}

		return chained_future->wait_until(timeout_time);
	}

	void loop_until_ready() const;

public:
	task_future_impl() = default;

	/**
	*	@brief	Chain a future with this one. Used for then_on_main_thread().
	*
	*	@param f	Future to chain. Will be moved from.
	*/
	template <bool b>
	task_future_impl(task_future_impl<chained_task_future, b> &&f,
					 task_future_chaining_construct) : sched(f.sched),
													   chain(true),
													   chaining_future(std::move(f.future)) {
		assert(!f.chain && "Can not double chain task_futures");
	}
	/**
	*	@brief	Chain a future with this one. Used for then_on_main_thread().
	*
	*	@param f	Future to chain. Will be moved from.
	*/
	task_future_impl(const task_future_impl<chained_task_future, true> &f,
					 task_future_chaining_construct) : sched(f.sched),
													   chain(true),
													   chaining_future(f.future) {
		assert(!f.chain && "Can not double chain task_futures");
	}

	task_future_impl(task_future_impl &&other) : sched(other.sched),
												 future(std::move(other.future)),
												 chain(other.chain),
												 chaining_future(std::move(other.chaining_future)),
												 chained_future(std::move(other.chained_future)) {}
	template <bool b = is_shared>
	task_future_impl(const task_future_impl &other, std::enable_if_t<b>* = nullptr) : sched(other.sched),
																					  future(other.future),
																					  chain(other.chain),
																					  chaining_future(other.chaining_future),
																					  chained_future(other.chained_future) {}

	task_future_impl &operator=(task_future_impl &&other) {
		sched = other.sched;
		future = std::move(other.future);
		chain = other.chain;
		chaining_future = std::move(other.chaining_future);
		chained_future = std::move(other.chained_future);

		return *this;
	}
	template <bool b = is_shared>
	std::enable_if_t<b, task_future_impl&> operator=(const task_future_impl &other) {
		sched = other.sched;
		future = other.future;
		chain = other.chain;
		chaining_future = other.chaining_future;
		chained_future = other.chained_future;

		return *this;
	}

	~task_future_impl() noexcept {}

	/**
	*	@brief	Get future return. Will busy wait if called on main thread.
	*/
	R get() {
		if (is_main_thread())
			loop_until_ready();

		if (chain)
			return chained_get();
		return future.get();
	}

	/**
	*	@brief	Wait for future. Will busy wait if called on main thread.
	*/
	void wait() const {
		if (is_main_thread())
			loop_until_ready();

		if (chain)
			return chained_wait();
		return future.wait();
	}

	/**
	*	@brief	Wait for future for limited duration. Can not be called on main thread unless timeout_duration is 0.
	*
	*	@param	timeout_duration	Timeout
	*/
	template <class Rep, class Period>
	std::future_status wait_for(const std::chrono::duration<Rep,Period> &timeout_duration) const {
		assert((std::chrono::duration_cast<std::chrono::microseconds>(timeout_duration) <= std::chrono::microseconds(0) ||
				!is_main_thread()) && "Blocking main thread");

		if (chain)
			return chained_wait_for(timeout_duration);
		return future.wait_for(timeout_duration);
	}

	/**
	*	@brief	Wait for future until a time point. Can not be called on main thread.
	*
	*	@param	timeout_time	Timeout time point
	*/
	template <class Clock, class Duration>
	std::future_status wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		assert(!is_main_thread() && "Blocking main thread");

		if (chain)
			return chained_wait_until(timeout_time);
		return future.wait_until(timeout_time);
	}

	bool valid() const { return future.valid(); }

	/**
	*	@brief	Schedules a lambda after this future's completion. Moves from this future and creates a new future.
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	task_future_impl<typename function_traits<L>::result_t, is_shared> then(L &&lambda) &&;
	/**
	*	@brief	Schedules a lambda on the main thread after this future's completion. Moves from this future and creates a new future.
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	task_future_impl<typename function_traits<L>::result_t, is_shared> then_on_main_thread(L &&lambda) &&;

	/**
	*	@brief	Moves this future into a shared future.
	*/
	task_future_impl<R, true> shared() &&;
};

template <typename R>
using task_future = task_future_impl<R, false>;
template <typename R>
using task_shared_future = task_future_impl<R, true>;

template <typename R>
using task_future_collection = future_collection<R, task_future>;

}
