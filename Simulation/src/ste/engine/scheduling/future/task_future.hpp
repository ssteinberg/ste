// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <function_traits.hpp>
#include <thread_constants.hpp>

#include <future_collection.hpp>

#include <functional>
#include <memory>
#include <atomic>

#include <future>
#include <mutex>
#include <shared_mutex>

#include <chrono>

#include <type_traits>

namespace StE {

class task_scheduler;

/**
 *	@brief	Thread-safe wrapper around std::future. Used by StE::task_scheduler.
 *			For thread safety task_future employs finely-grained read-write locks
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
	using future_type 			= typename std::conditional<is_shared, std::shared_future<R>, std::future<R>>::type;

	using mutex_type		= std::shared_timed_mutex;
	using read_lock_type 	= std::shared_lock<mutex_type>;
	using write_lock_type 	= std::unique_lock<mutex_type>;

private:
	template <typename lock_type>
	class future_lock_guard {
		lock_type ul;

	public:
		future_lock_guard(mutex_type &m) : ul(m, std::defer_lock) {
			ul.lock();
		}
	};

private:
	mutable mutex_type mutex;

	task_scheduler *sched;
	future_type future;

private:
	// Helper get/wait methods. Try to lock read mutex and get or wait for future using specified timeouts.
	template <typename Future>
	auto future_get(Future &f) {
		write_lock_type rl(mutex);
		return f.get();
	}
	template <typename Future>
	auto future_wait(const Future &f) const {
		read_lock_type rl(mutex);
		return f.wait();
	}
	template <typename Future, class Rep, class Period>
	auto future_wait_for(const Future &f, std::chrono::duration<Rep,Period> timeout_duration) const {
		auto start = std::chrono::high_resolution_clock::now();

		read_lock_type rl(mutex, std::defer_lock);
		if (!rl.try_lock_for(timeout_duration))
			return std::future_status::timeout;

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<decltype(timeout_duration)>(end - start);
		timeout_duration -= elapsed;

		return f.wait_for(timeout_duration);
	}
	template <typename Future, class Clock, class Duration>
	auto future_wait_until(const Future &f, const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		read_lock_type rl(mutex, std::defer_lock);
		if (!rl.try_lock_until(timeout_time))
			return std::future_status::timeout;

		return f.wait_until(timeout_time);
	}

private:
	task_future_impl(typename task_future_impl<R, false>::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}
	task_future_impl(typename task_future_impl<R, true >::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}

	task_future_impl(future_lock_guard<write_lock_type> &&l,
					 task_future_impl &&other) : sched(other.sched), future(std::move(other.future)) {}

	template <bool b = is_shared>
	task_future_impl(const typename task_future_impl<R, true>::future_type &f,
					 task_scheduler *sched,
					 std::enable_if_t<b>* = nullptr) : sched(sched), future(f) {}

	template <bool b = is_shared>
	task_future_impl(future_lock_guard<read_lock_type> &&l,
					 const task_future_impl &other,
					 std::enable_if_t<b>* = nullptr) : sched(other.sched),
													   future(other.future) {}

public:
	task_future_impl() = default;

	/**
	*	@brief	Move ctor.
	*
	*	@param other	Future to move.
	*/
	task_future_impl(task_future_impl &&other) noexcept : task_future_impl(future_lock_guard<write_lock_type>(other.mutex),
																		   std::move(other)) {}
	/**
	*	@brief	Copy ctor.
	*
	*	@param other	Future to copy. Must be a shared_future.
	*/
	template <bool b = is_shared>
	task_future_impl(const task_future_impl &other,
					 std::enable_if_t<b>* = nullptr) : task_future_impl(future_lock_guard<read_lock_type>(other.mutex),
																		other) {}

	~task_future_impl() noexcept {}

	task_future_impl &operator=(const task_future_impl &other) = delete;

	/**
	*	@brief	Move operator.
	*
	*	@param other	Future to move.
	*/
	task_future_impl &operator=(task_future_impl &&other) noexcept {
		write_lock_type l0(mutex);
		write_lock_type l1(other.mutex);

		std::lock(l0, l1);

		sched = other.sched;
		future = std::move(other.future);

		return *this;
	}

	/**
	*	@brief	Get future return.
	*/
	R get() {
		return future_get(future);
	}

	/**
	*	@brief	Wait for future.
	*/
	void wait() const {
		future_wait(future);
	}

	/**
	*	@brief	Wait for future for limited duration. 
	*
	*	@param	timeout_duration	Timeout
	*/
	template <class Rep, class Period>
	std::future_status wait_for(const std::chrono::duration<Rep,Period> &timeout_duration) const {
		return future_wait_for(future, timeout_duration);
	}

	/**
	*	@brief	Wait for future until a time point.
	*
	*	@param	timeout_time	Timeout time point
	*/
	template <class Clock, class Duration>
	std::future_status wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		return future_wait_until(future, timeout_time);
	}

	/**
	*	@brief	Schedules a lambda after this future's completion. Moves from this future and creates a new future.
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	task_future_impl<typename function_traits<L>::result_t, is_shared> then(L &&lambda) &&;

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
